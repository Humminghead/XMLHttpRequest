#include "xmlhttprequest.h"

#include <spdlog/spdlog.h>

#include <functional>
#include <memory>

#include "httpheadervalue.h"
#include "httprequest.h"
#include "httpresponce.h"
#include "httptlssession.h"
#include "jsfunctions.h"
#include "sessionstream.h"
#include "types.h"
#include "xmlhttprequest_impl.h"

namespace network {

XMLHttpRequest::XMLHttpRequest() : d{new Impl(), [](auto *p) { delete p; }} {
  spdlog::debug("{} Create new XMLHttpRequest()", pthread_self());
}

XMLHttpRequest::XMLHttpRequest(std::string &&method, std::string &&url,
                               bool async)
    : d{new Impl(), [](auto *p) { delete p; }} {

  spdlog::debug("{} Create new XMLHttpRequest({}, {}, {})", pthread_self(),
                method, url, async);

  if (auto ok = this->prepare(std::move(method), std::move(url), async); !ok)
    spdlog::error("{} Error while session prepare", pthread_self());

  if (auto ok = setup(); !ok)
    spdlog::error("{} Error while session setup", pthread_self());
}

XMLHttpRequest::XMLHttpRequest(const std::string_view method,
                               const std::string_view url, bool async)
    : d{new Impl(), [](auto *p) { delete p; }} {

  spdlog::debug("{} Create new XMLHttpRequest({}, {}, {})", pthread_self(),
                method, url, async);

  if (auto ok = this->prepare(method.data(), url.data(), async); !ok)
    spdlog::error("{} Error while session prepare", pthread_self());

  if (auto ok = setup(); !ok)
      spdlog::error("{} Error while session setup", pthread_self());
}

XMLHttpRequest::XMLHttpRequest(XMLHttpRequest &&request):d{std::move(request.d)}{}

XMLHttpRequest &XMLHttpRequest::operator()(XMLHttpRequest &&request){
    this->d = std::move(request.d);
    return *this;
}

XMLHttpRequest::~XMLHttpRequest()
{
    this->abort();
}

void XMLHttpRequest::abort() {
    spdlog::trace("{} XMLHttpRequest::abort()", pthread_self());
  if (!d) {
      spdlog::error("{} XMLHttpRequest abort failed! Request is empty!", pthread_self());
      return ;
  }

  d->service->stop();
  d->session->stop();
}

std::string XMLHttpRequest::getAllResponseHeaders() {

  std::string headers{""};

  if (auto resp = d->responce; resp) {
    for (auto &header : resp->header()) {
      auto [name, data] = header;
      headers.append(name + ": " + data.value + "\r\n");
    }
  }

  return headers;
}

std::string XMLHttpRequest::getResponseHeader(const std::string &header) {
  if (auto resp = d->responce; resp) {
    auto it = std::find_if(std::begin(resp->header()), std::end(resp->header()),
                           [&](auto &hdr) { return hdr.first == header; });

    if (it != std::end(resp->header()))
      return it->second.value;
  }

  return "";
}

bool XMLHttpRequest::open() {
  spdlog::trace("{} XMLHttpRequest::open()", pthread_self());

  if(!d){
      spdlog::error("{} XMLHttpRequest open failed! Request is empty!", pthread_self());
      return false;
  }

  if (d->readyState >= ReadyState::Opened) {
    spdlog::error("{} Session alredy open!", pthread_self());
    return false;
  }

  if (bool isOk = setup(); !isOk)
    spdlog::warn("{} Setup alredy performed", pthread_self());

  setupTimer();

  if (d->session) {
    if (auto cb = d->session->getOnStateChangeCallback(); cb)
      cb(static_cast<uint8_t>(ReadyState::Opened));
  }

  return true;
}

void XMLHttpRequest::open(std::string &&method, std::string &&url) {
  this->open(std::move(method), std::move(url), false);
}

void XMLHttpRequest::open(std::string &&method, std::string &&uri, bool async) {
  spdlog::trace("{} XMLHttpRequest::open({}, {}, {})", pthread_self(), method,
                uri, async);

  if (auto isOk = prepare(std::move(method), std::move(uri), async); !isOk) {
    spdlog::error("{} Error while session prepare", pthread_self());
    return;
  } else {
    if (!this->open()) {
      spdlog::error("{} Error while session setup", pthread_self());
      return;
    }
  }
}

void XMLHttpRequest::open(std::string method,     //
                          std::string uri,        //
                          bool async,             //
                          const std::string user, //
                          const std::string password) {
  this->open(std::move(method), std::move(uri), async);

  ArrayBuffer<> buffer{};
  buffer.resize(user.size() + password.size() + 1);

  auto buffIt = std::begin(buffer);

  std::copy(std::begin(user), std::end(user), buffIt);
  buffIt += user.size();

  *buffIt = ':';
  buffIt += 1;

  if (buffIt != std::end(buffer))
    std::copy(std::begin(password), std::end(password), buffIt);

  // clang-format off
  setRequestHeader("Authorization", "Basic " +jsobjects::encodeFromArray(std::move(buffer)));
  // clang-format on
}

void XMLHttpRequest::overrideMimeType(std::string &&mime) {
  d->session->setMimeOverridenType(std::move(mime));
}

void XMLHttpRequest::send() {
  if (!d) {
      spdlog::error("{}  XMLHttpRequest::send() failed! Request is empty!", pthread_self());
      return ;
  }

  if (auto method = HttpTlsSession::methodFromString(d->method);
      method == HttpTlsSession::Method::Get) {
    sendGet();
  }
  else if(method == HttpTlsSession::Method::Post){
    sendPost("");
  }else {
    spdlog::info("{} Unsupported method \"{}\" in XMLHttpRequest::send() call!",
                 pthread_self(), d->method);
  }
}

void XMLHttpRequest::send(onReadyCallback &&cb) {
  if (!d->session) {
    spdlog::error("{} Session is NULL! The request has not been opened!",
                  pthread_self());
    return;
  }

  setOnReadyCallback(std::move(cb));
  send();
}

void XMLHttpRequest::send(std::string &&body) {
  this->sendPost(std::move(body));
}

void XMLHttpRequest::setRequestHeader(const std::string &header,
                                      const std::string &value) {
  d->header.emplace(header, HeaderValue{value.c_str(), value.size(), false});
}

void XMLHttpRequest::setRequestHeader(std::string &&header,
                                      std::string &&value) {
  d->header.emplace(std::move(header), HeaderValue{std::move(value), false});
}

void XMLHttpRequest::openRequest(std::string method, //
                                 std::string url,    //
                                 bool async,         //
                                 std::string user,   //
                                 std::string password) {
    open(std::move(method), std::move(url), std::move(async), std::move(user), std::move(password));
}

bool XMLHttpRequest::setOnReadyCallback(onReadyCallback &&cb) noexcept {

  if (!d->session)
    return false;

  d->session->setOnReadyCallback(
      [=](std::pair<std::shared_ptr<Request>, std::shared_ptr<Responce>>
              &&pair) {
        // Save responce
        d->responce = pair.second;

        // Invoke callback function
        if (cb)
          cb(std::move(pair));

        if (!d->isAsync)
          d->session->stop();
      });

  return true;
}

bool XMLHttpRequest::setOnStateChangedCallback(
    onStateChangeCallback &&cb) noexcept {
  if (!d->session)
    return false;

  d->session->setOnStateChangeCallback([=](const uint8_t state) {
    // Set current state
    d->readyState = static_cast<ReadyState>(state);

    // Invoke outer callback
    if (cb)
      cb(static_cast<uint8_t>(d->readyState));
  });

  return true;
}

void XMLHttpRequest::setOnTimeoutCallback(onTimeoutCallback &&cb) noexcept {
  d->timerCallback = std::move(cb);
}

void XMLHttpRequest::timeout(const size_t milliseconds) noexcept {
  if (d->readyState != ReadyState::Unsent) {
    spdlog::error("{} Timeout isn't set! Beacause request already opened!",
                  pthread_self());
    return;
  }
  spdlog::debug("{} Timeout was set to {} milliseconds! ", pthread_self(),
                milliseconds);
  d->timeout = std::chrono::milliseconds(milliseconds);
}

size_t XMLHttpRequest::timeout() const { return d->timeout.count(); }

std::shared_ptr<Responce> XMLHttpRequest::responce() const {
  return d->responce;
}

void XMLHttpRequest::initCallbacks() noexcept {
  if (d->readyState != ReadyState::Unsent)
    return;

  if (!d->session->getOnStateChangeCallback()) {
    d->session->setOnStateChangeCallback([&](const uint8_t state) {
      spdlog::trace("{} Invoke onStateChangeCallback in "
                    "XMLHttpRequest::initCallbacks(), set state: {}",
                    pthread_self(), readyStateToString(state));
      d->readyState = static_cast<ReadyState>(state);
    });
  }

  if (!d->session->getOnStopCallback())
    d->session->setOnStopCallback([&] {
      d->service->stop();
      d->service->reset();
    });
}

bool XMLHttpRequest::prepare(std::string &&method, std::string &&uri,
                             bool async) {

  std::transform(std::begin(method), std::end(method), std::begin(method),
                 [](unsigned char c) { return std::toupper(c); });

  if(!d){
      spdlog::error("{} XMLHttpRequest::prepare failed! Request empty!", pthread_self());
      return false;
  }

  d->isAsync = async;

  auto [ok, elements] = d->proccessUri(uri);

  if (ok) {
    d->addr = std::move(elements.addr);
    d->port = std::move(elements.port);
    d->proto = std::move(elements.protocol);
    d->urn = std::move(elements.urn);
    d->method = std::move(method);

    return true;
  }

  return false;
}

bool XMLHttpRequest::setup() {

  bool ok{false};

  if (!d->session) {
    ok = d->setupSession();
    if (ok) {
      this->initCallbacks();
    }
  }

  return ok;
}

void XMLHttpRequest::setupTimer() noexcept {
  if (auto interval = d->timeout.count(); interval > 0) {
    spdlog::debug("{} Setup XMLHttpRequest timer with interval {} ms",
                  pthread_self(), interval);

    if (d->timer) {
      d->timer->cancel();
      d->timer.reset();
    }

    d->timer = std::make_shared<boost::asio::steady_timer>(
        d->service->get_executor(), d->timeout);

    d->timer->async_wait([&](const boost::system::error_code &code) {
      spdlog::debug(
          "{} XMLHttpRequest timeout callback invoked!(Error code: {})",
          pthread_self(), code.message());
      if (!code.failed()) {
        if (auto cb = d->timerCallback; cb)
          cb();

        this->abort();
      }
    });
  }
}

void XMLHttpRequest::sendGet() {
  // clang-format on
  if (!d->service || !d->session /*|| !d->future*/)
    return;

  // Reset responce
  d->responce.reset();

  // Put jobs into io_service
  d->service->post([&] { this->connect(); });
  d->service->post([&] { this->fetch(); });

  // Choose mode
  if (d->isAsync) { // ASYNC
    d->future.reset();
    d->future = std::make_shared<XMLHttpRequest::Impl::future_t>(
        std::async(std::launch::async,
                   // clang-format off
                         // { ret} | { pointer to class of function } | { function that id needed }
                           (size_t(boost::asio::io_service::*)(void))& boost::asio::io_service::run,
                   // clang-format on
                   d->service.get()));
  } else { // SYNC
    ///\todo deadline timer after what GOAWAY must be sended
    d->service->run();
  }
}

void XMLHttpRequest::sendPost(std::string &&body) {
  spdlog::trace("{} XMLHttpRequest::sendPost()", pthread_self());

  if (!d->service || !d->session)
    return;

  // Save boby
  d->body = std::move(body);

  // Reset responce
  d->responce.reset();

  // Put jobs into io_service
  d->service->post([&] { this->connect(); });
  d->service->post([&] { this->fetchWithBody(d->body); });

  // Choose mode
  if (d->isAsync) { // ASYNC
    d->future.reset();
    d->future = std::make_shared<XMLHttpRequest::Impl::future_t>(
        std::async(std::launch::async,
                   // clang-format off
                         // { ret} | { pointer to class of function } | { function that id needed }
                           (size_t(boost::asio::io_service::*)(void))& boost::asio::io_service::run,
                   // clang-format on
                   d->service.get()));
  } else { // SYNC
    ///\todo deadline timer after what GOAWAY must be sended
    d->service->run();
  }
}

void XMLHttpRequest::connect() {
  spdlog::trace("{} Perform \"XMLHttpRequest::connect()\" function",
                pthread_self());

  if (d->session->isStopped()) {
    spdlog::trace("{} Do session->startResolve()", pthread_self());
    d->session->startResolve();
  }
}

void XMLHttpRequest::fetch() {
  spdlog::trace("{} Perform \"XMLHttpRequest::fetch()\" function",
                pthread_self());

  if (d->session->isStopped()) {
    spdlog::trace("{} \"XMLHttpRequest::fetch()\" Session stoppped. Exit!",
                  pthread_self());
    return;
  }

  if (auto id = d->fetch(d->method,
                         d->proto + "://" + d->addr + ":" + d->port + d->urn);
      id < 0) {
    spdlog::debug("{} Session stoped because error happens!", pthread_self());
    d->session->stop();
  }
}

void XMLHttpRequest::fetchWithBody(const std::string &body_) {
  spdlog::trace("{} Perform \"XMLHttpRequest::fetchWithBody()\" function",
                pthread_self());
  if (auto id =
          d->fetch(d->method,
                   d->proto + "://" + d->addr + ":" + d->port + d->urn, body_);
      id < 0) {
    spdlog::debug("{} Session stoped because error happens!", pthread_self());
    d->session->stop();
  }
}

} // namespace network
