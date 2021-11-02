#include "abstractsession.h"

#include "defaults.h"
#include "httpheader.h"
#include "httpheadervalue.h"
#include "httprequest.h"
#include "httpresponce.h"
#include "sessioncallbacks.h"
#include "sessionstream.h"
#include "sessionutil.h"
#include "utilities.h"

#include <array>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <nghttp2/nghttp2.h>
#include <spdlog/spdlog.h>
#include <string>

namespace network {
using DeadLineTimer = boost::asio::deadline_timer;
using PosixTimeDuration = boost::posix_time::time_duration;

using namespace network::literals;

[[maybe_unused]] auto sendClientConnectionHeader =
    [](nghttp2_session *session) {
      // clang-format off
      constexpr static std::array<nghttp2_settings_entry, 2> iv = {
      nghttp2_settings_entry{NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS,defaults::http2_max_streams},
      nghttp2_settings_entry{NGHTTP2_SETTINGS_MAX_FRAME_SIZE,defaults::http2_max_frame}};
      // clang-format on
      /* client 24 bytes magic string will be sent by nghttp2 library */
      return nghttp2_submit_settings(session, NGHTTP2_FLAG_NONE, &(iv.front()),
                                     iv.size());
    };

struct AbstractSession::Impl {
  Impl(boost::asio::io_service &srv, const PosixTimeDuration &connectTimeout,
       const PosixTimeDuration &readTimeout)
      : service(srv), deadline(service), ping(service),
        connect_timeout_(connectTimeout), read_timeout_(readTimeout),
        resolver_(service) {}

  boost::asio::io_service &service;
  ErrorCode ec;

  std::mutex nghttp2_session_mutex_;
  nghttp2_session *session{nullptr};

  DeadLineTimer deadline;
  DeadLineTimer ping;

  std::atomic_bool pingIsStarted;
  std::atomic_bool deadlineIsStarted;

  PosixTimeDuration connect_timeout_;
  PosixTimeDuration read_timeout_;

  TcpResolver resolver_;

  Array<64_Kb> rb_;
  Array<64_Kb> wb_;

  std::size_t wblen_;

  bool stopped{true};

  bool writing{false};

  onReadyCallback onReadyCb;
  onStopCallback onStopCb;
  onConnectCallback onStartCb;
  onStateChangeCallback onReadyStateChange;

  http_parser_url parser{};

  Streams streams;

  std::mutex insideCb;

  const uint8_t *data_pending_{nullptr};
  size_t data_pendinglen_{0};

  SessionHolder abstractSessionHolder;

  bool setupSession() {
    std::lock_guard lock(nghttp2_session_mutex_);

    if (session)
      return false;

    nghttp2_session_callbacks *callbacks;
    nghttp2_session_callbacks_new(&callbacks);
    auto cb_del =
        utils::commands::defer(nghttp2_session_callbacks_del, callbacks);

    nghttp2_session_callbacks_set_on_begin_headers_callback(
        callbacks, callbacks::onBeginHeadersCallback);

    nghttp2_session_callbacks_set_on_header_callback(
        callbacks, callbacks::onHeaderCallback);

    nghttp2_session_callbacks_set_on_frame_recv_callback(
        callbacks, callbacks::onFrameRecvCallback);

    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(
        callbacks, callbacks::onDataChunkRecvCallback);

    nghttp2_session_callbacks_set_on_stream_close_callback(
        callbacks, callbacks::onStreamCloseCallback);

    nghttp2_session_callbacks_set_before_frame_send_callback(
        callbacks, callbacks::beforeFrameSendCallback);

    if (auto rv = nghttp2_session_client_new(&session, callbacks,
                                             &abstractSessionHolder);
        rv != 0) {
      return false;
    }

    return true;
  }

  std::string overrideMimeType_{};
};

AbstractSession::AbstractSession(
    boost::asio::io_service &io_service,
    const boost::posix_time::time_duration &connect_timeout,
    const boost::posix_time::time_duration &read_timeout)
    : d{new Impl(io_service, connect_timeout, read_timeout),
        [](auto *p) { delete p; }} {
  d->abstractSessionHolder = [this] { return shared_from_this(); };
}

AbstractSession::~AbstractSession() { nghttp2_session_del(d->session); }

void AbstractSession::startResolve(const std::string &host,
                                   const std::string &service,
                                   ErrorCode &ec) noexcept {

  auto endpoint = d->resolver_.resolve(host, service, ec);

  if (ec) {
    spdlog::debug("{} {}", pthread_self(), ec.message());
    stop();
    return;
  }

  // Connect
  if (this->startConnect(endpoint, ec); ec.failed()) {
    stop();
    return;
  }

  //  waitConnection();
  onSocketConnected(endpoint);

  // Invoke callback onStart
  if (auto cb = getOnConnectCallback(); cb)
    cb();
}

void AbstractSession::stop() {
  spdlog::trace("{} AbstractSession::stop()", pthread_self());

  if (isStopped()) {
    spdlog::trace("{} Already stopped", pthread_self());
    return;
  }

  spdlog::trace("{} Shutdown socket", pthread_self());
  cancelPing();
  shutdownSocket();
  d->deadline.cancel();
  setStopped(true);

  if (d->onStopCb)
    onStopCallback();
}

void AbstractSession::read() noexcept {
  spdlog::trace("{} AbstractSession::read()", pthread_self());

  if (isStopped()) {
    spdlog::trace("{} Session stopped - exit!", pthread_self());
    return;
  }

  auto self = shared_from_this();

  auto readSocketPredicate = [self](const ErrorCode &ec,
                                    std::size_t bytes_transferred) {
    spdlog::trace("{} readSocketPredicate(ec, {})", pthread_self(),
                  bytes_transferred);

    if (bytes_transferred == 0) {
      spdlog::trace("{} Bytes transferred - exit!", pthread_self());
      return;
    }

    if (ec.failed()) {
      spdlog::error("{} {}", pthread_self(), ec.message());
      self->stop();
      return;
    }

    {
      std::lock_guard<decltype(self->d->insideCb)> lock(self->d->insideCb);
      if (auto rv = nghttp2_session_mem_recv(
              self->d->session, self->d->rb_.data(), bytes_transferred);
          rv != static_cast<ssize_t>(bytes_transferred)) {
        self->stop();
        spdlog::error("{} nghttp2_session_mem_recv code: {} - exit!",
                      pthread_self(), rv);
        return;
      }
    }

    self->write();

    if (self->shouldStop()) {
      spdlog::trace("{} Session should stop - exit!", pthread_self());
      self->stop();
      return;
    }

    self->read();
  };

  readSocket(readSocketPredicate);
}

void AbstractSession::write() noexcept {
  spdlog::trace("{} AbstractSession::write()", pthread_self());

  if (isStopped()) {
    spdlog::trace("{} Session stopped - exit!", pthread_self());
    return;
  }

  if (d->writing) {
    spdlog::trace("{} Writing now - exit!", pthread_self());
    return;
  }

  if (d->data_pending_) {
    spdlog::trace("{} Copy {} bytes of data in WB", pthread_self(),
                  d->data_pendinglen_);
    std::copy_n(d->data_pending_, d->data_pendinglen_,
                std::begin(d->wb_) + d->wblen_);

    d->wblen_ += d->data_pendinglen_;

    d->data_pending_ = nullptr;
    d->data_pendinglen_ = 0;
  }
  {
    std::lock_guard<decltype(d->insideCb)> lock(d->insideCb);
    for (ssize_t n = 0;;) {
      const uint8_t *data{nullptr};

      n = nghttp2_session_mem_send(d->session, &data);

      spdlog::trace("{} nghttp2_session_mem_send() return {}", pthread_self(),
                    n);

      if (NGHTTP2_ERR_NOMEM == n) {
        spdlog::critical("{} Out of memory - exit!", pthread_self());
        stop();
        return;
      }

      if (n == 0) {
        spdlog::trace("{} No data is available to send - break!",
                      pthread_self());
        break;
      }

      if (d->wblen_ + n > d->wb_.size()) {
        d->data_pending_ = data;
        d->data_pendinglen_ = n;

        break;
      }

      std::copy_n(data, n, std::begin(d->wb_) + d->wblen_);

      d->wblen_ += n;
    }
  }

  if (d->wblen_ == 0) {
    spdlog::trace("{} WB lenght is 0 - exit!", pthread_self());
    return;
  }

  d->writing = true;

  // Reset read deadline here, because normally client is sending
  // something, it does not expect timeout while doing it.

  auto self = this->shared_from_this();

  writeSocket([self](const ErrorCode &ec, std::size_t n) {
    spdlog::trace("{} writeSocket()", pthread_self());
    (void)n;

    if (ec) {
      spdlog::error("{}{}{}", pthread_self(), ec.value(), ec.message());
      self->stop();
      return;
    }

    std::fill_n(std::begin(self->d->wb_), self->d->wblen_, 0x0);
    self->d->wblen_ = 0;
    self->d->writing = false;

    self->write();
  });
}

bool AbstractSession::isStopped() const noexcept { return d->stopped; }

void AbstractSession::setStopped(bool stopped) noexcept {
  d->stopped = stopped;
}

AbstractSession::ErrorCode &AbstractSession::getError() const { return d->ec; }

void AbstractSession::onSocketConnected(
    AbstractSession::EndpointIt endpoint_it) {
  (void)endpoint_it;

  ///\todo add error code
  setStopped(false);

  if (!d->setupSession()) {
    d->ec.assign(NGHTTP2_ERR_FATAL, NetworkErrorCategory{});
    return;
  }

  socket().set_option(boost::asio::ip::tcp::no_delay(true));

  sendClientConnectionHeader(d->session);

  write();
  read();

  startPing();
}

void AbstractSession::setOnReadyCallback(onReadyCallback &&cb) {
  if (cb) {
    d->onReadyCb = std::move(cb);
  }
}

onReadyCallback &AbstractSession::getOnReadyCallback() const {
  return d->onReadyCb;
}

void AbstractSession::setOnStopCallback(onStopCallback &&cb) {
  d->onStopCb = std::move(cb);
}

onStopCallback &AbstractSession::getOnStopCallback() const {
  return d->onStopCb;
}

void AbstractSession::setOnConnectCallback(onConnectCallback &&cb) {
  d->onStartCb = std::move(cb);
}

onConnectCallback &AbstractSession::getOnConnectCallback() const {
  return d->onStartCb;
}

void AbstractSession::setOnStateChangeCallback(onStateChangeCallback &&cb) {
  d->onReadyStateChange = std::move(cb);
}

onStateChangeCallback &AbstractSession::getOnStateChangeCallback() const {
  return d->onReadyStateChange;
}

const char *NetworkErrorCategory::name() const noexcept {
  return "network_module_error";
}

std::string NetworkErrorCategory::message(int ev) const {
  return "nghttp2 network error: " + std::to_string(ev);
}

std::tuple<AbstractSession::ErrorCode, int32_t>
AbstractSession::submit(std::string_view url,    //
                        std::string_view method, //
                        const Header &header,    //
                        int32_t &streamId,       //
                        int32_t &weightNum,      //
                        const bool isExclusive,  //
                        const void *dataProvider) noexcept {
  if (isStopped()) {
    return {
        {static_cast<nghttp2_error>(NGHTTP2_ERR_FATAL), NetworkErrorCategory{}},
        -1};
  }

  if (http_parser_parse_url(url.data(), url.size(), 0, &d->parser) != 0) {
    return {make_error_code(boost::system::errc::invalid_argument), -1};
  }

  if ((d->parser.field_set & (1 << UF_SCHEMA)) == 0 ||
      (d->parser.field_set & (1 << UF_HOST)) == 0) {
    return {make_error_code(boost::system::errc::invalid_argument), -1};
  }

  std::string scheme{}, host{}, rawPath{}, rawQuery{}, path{};

  session_utils::copy_url_component(scheme, &d->parser, UF_SCHEMA, url.data());
  session_utils::copy_url_component(host, &d->parser, UF_HOST, url.data());
  session_utils::copy_url_component(rawPath, &d->parser, UF_PATH, url.data());
  session_utils::copy_url_component(rawQuery, &d->parser, UF_QUERY, url.data());

  if (session_utils::ipv6_numeric_addr(host.data())) {
    host = "[" + host + ']';
  }
  if (d->parser.field_set & (1 << UF_PORT)) {
    host += ':';
    host += session_utils::utos(d->parser.port);
  }

  if (rawPath.empty()) {
    rawPath = "/";
  }

  path = session_utils::percent_decode(std::begin(rawPath), std::end(rawPath));
  path = rawPath;

  if (d->parser.field_set & (1 << UF_QUERY)) {
    path += '?';
    path += rawQuery;
  }

  auto nva = std::vector<nghttp2_nv>();
  nva.reserve(4 + header.size());
  nva.push_back(session_utils::make_nv_ls(":method", method.data()));
  nva.push_back(session_utils::make_nv_ls(":scheme", scheme));
  nva.push_back(session_utils::make_nv_ls(":path", path));
  //  nva.push_back(session_utils::make_nv_ls(":authority", host));

  for (auto &kv : header) {
    nva.push_back(
        session_utils::make_nv(kv.first, kv.second.value, kv.second.sensitive));
  }

  nghttp2_priority_spec prio{};

  nghttp2_priority_spec_init(&prio, streamId, weightNum, isExclusive);

  // Create new stream
  std::shared_ptr<Stream> strm{};

  auto assignedId =
      nghttp2_submit_request(d->session,                            //
                             &prio,                                 //
                             nva.data(),                            //
                             nva.size(),                            //
                             (nghttp2_data_provider *)dataProvider, //
                             &strm);

  if (assignedId < 0) {
    return {{static_cast<nghttp2_error>(NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE),
             NetworkErrorCategory{}},
            assignedId};
  }

  if (auto locked = d->insideCb.try_lock(); locked) {
    write();
    d->insideCb.unlock();
  }

  strm = std::make_shared<Stream>(
      assignedId, nghttp2_session_find_stream(d->session, assignedId));

  // addStream
  d->streams.try_emplace(assignedId, std::move(strm));

  return {ErrorCode{}, assignedId};
}

void AbstractSession::startPing() noexcept {
  spdlog::trace("{} AbstractSession::startPing()", pthread_self());

  d->ping.expires_from_now(boost::posix_time::seconds(defaults::ping_time));
  auto pinger = [this](const ErrorCode &ec) {
    submitPing(ec);
    write();
    startPing();
  };

  d->ping.async_wait(
      std::bind([pinger](auto ec) { pinger(ec); }, std::placeholders::_1));

  d->pingIsStarted.store(true);
}

size_t AbstractSession::cancelPing() noexcept {
  d->pingIsStarted.store(false);
  return d->ping.cancel();
}

AbstractSession::Array<64_Kb> &AbstractSession::getReadBuffer() noexcept {
  return d->rb_;
}

AbstractSession::Array<64_Kb> &AbstractSession::getWriteBuffer() noexcept {
  return d->wb_;
}

size_t AbstractSession::getWriteBufferSize() const noexcept {
  return d->wblen_;
}

void AbstractSession::startWaitConnectionTimer() const {
  ///\todo make it async
  d->deadline.expires_from_now(d->connect_timeout_);
  d->deadline.wait();
}

void AbstractSession::startReadTimer() const {
  auto onWait = [&](auto &ec) {
    (void)ec;
    { d->stopped = true; }
  };

  d->deadline.expires_from_now(d->read_timeout_);
  d->deadline.async_wait(onWait);
}

void AbstractSession::setMimeOverridenType(std::string &&mime) {
  d->overrideMimeType_ = std::move(mime);
}

const std::string &AbstractSession::getMimeOverridenType() const {
  return d->overrideMimeType_;
}

int AbstractSession::submitPing(const ErrorCode &ec) const {
  if (isStopped() || ec == boost::asio::error::operation_aborted) {
    spdlog::trace("{} Ping aborted!", pthread_self());
    return 0;
  }
  spdlog::trace("{} Submit ping", pthread_self());
  return nghttp2_submit_ping(d->session, NGHTTP2_FLAG_NONE, nullptr);
}
int AbstractSession::submitPing() const {
  ErrorCode ec;
  return submitPing(ec);
}

bool AbstractSession::shouldStop() const {
  return !d->writing && !nghttp2_session_want_read(d->session) &&
         !nghttp2_session_want_write(d->session);
}

void AbstractSession::goAway() noexcept {
  // Finish up all active streams
  for (auto &ptr : d->streams) {
    auto &strm = ptr.second;

    nghttp2_submit_goaway(d->session, NGHTTP2_FLAG_NONE, strm->id(),
                          NGHTTP2_NO_ERROR, nullptr, 0);
  }
}

AbstractSession::ErrorCode AbstractSession::goAway(uint32_t streamId) noexcept {
  if (auto it = d->streams.find(streamId); it != d->streams.end()) {
    auto ret = nghttp2_submit_goaway(d->session, NGHTTP2_FLAG_NONE, streamId,
                                     NGHTTP2_NO_ERROR, nullptr, 0);
    return {ret, NetworkErrorCategory{}};
  }
  return {NGHTTP2_ERR_INVALID_STREAM_ID, NetworkErrorCategory{}};
}

std::shared_ptr<Stream>
AbstractSession::createPushStream(uint32_t streamId) noexcept {
  return std::make_shared<Stream>(
      streamId, nghttp2_session_find_stream(d->session, streamId));
}

bool AbstractSession::addStream(std::shared_ptr<Stream> streamPtr) noexcept {
  auto [it, ok] = d->streams.try_emplace(streamPtr->id(), std::move(streamPtr));
  (void)it;
  return ok;
}

std::shared_ptr<Stream>
AbstractSession::findStream(uint32_t streamId) noexcept {
  if (auto it = d->streams.find(streamId); it != d->streams.end())
    return it->second;

  return nullptr;
}

std::vector<StreamPtr> AbstractSession::findClosedStreams() noexcept {
  std::vector<StreamPtr> streams;
  streams.reserve(defaults::http2_max_streams);

  for (auto &it : d->streams) {
    auto [n, stream] = it;
    if (auto state = stream->state(); state == Stream::State::Closed)
      streams.push_back(stream);
  };
  return streams;
}

void AbstractSession::removeStream(uint32_t streamId) noexcept {
  d->streams.erase(streamId);
}

bool AbstractSession::streamsEmpty() noexcept { return d->streams.empty(); }

nghttp2_session *AbstractSession::rawSession() const { return d->session; }

} // namespace network
