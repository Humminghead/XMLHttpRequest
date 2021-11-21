#include "httptlssession.h"

#include "defaults.h"
#include "types.h"
#include "utilities.h"
// clang-format off
#include "httpheadervalue.h"
#include "httpheader.h"
// clang-format on
#include "sessionutil.h"
#include "netservice.h"

#include <deque>
#include <string_view>
#include <string>
#include <spdlog/spdlog.h>

static constexpr std::string_view methodGet{"GET"};
static constexpr std::string_view methodPost{"POST"};

namespace network {
struct HttpTlsSession::Impl {
  Impl(boost::asio::io_service &io_service, boost::asio::ssl::context &tlsCtx,
       const std::string &hst, const std::string &srv)
      : service_{io_service, tlsCtx, hst, srv} {}

  NetService service_;
};

auto implDeleter([](auto *p) {
  if (p)
    delete p;
});

HttpTlsSession::HttpTlsSession(boost::asio::io_service &io_service,
                               boost::asio::ssl::context &tlsCtx,
                               const std::string &host,
                               const std::string &service)
    : AbstractSession(
          io_service,
          boost::posix_time::seconds(defaults::http_connect_timeout),
          boost::posix_time::seconds(defaults::http_read_timeout)),
      d(new Impl(io_service, tlsCtx, host, service), implDeleter) {}

std::tuple<AbstractSession::ErrorCode, int32_t>
HttpTlsSession::submit(std::string_view url, std::string_view method,
                       Header &&header, int32_t &stream_id, int32_t &weight,
                       const bool exclusive = false) noexcept {
  return AbstractSession::submit(url, method, std::move(header), stream_id,
                                 weight, exclusive);
}

std::tuple<AbstractSession::ErrorCode, int32_t>
HttpTlsSession::submit(Header &&headers, std::string_view url,
                       Method method) noexcept {
  int32_t id{0};
  int32_t weight{0};

  return this->submit(url,                           //
                      stringFromEnum(method).data(), //
                      std::move(headers),            //
                      id,                            //
                      weight                         //
  );
}

std::tuple<AbstractSession::ErrorCode, int32_t>
HttpTlsSession::submit(Header &&headers, std::string_view url, Method method,
                       const std::string &body) noexcept {
  int32_t id{0};
  int32_t weight{0};
  auto methodStr = stringFromEnum(method);

  auto reciveCallback = [](nghttp2_session *session, int32_t stream_id,
                           uint8_t *buf, size_t length, uint32_t *data_flags,
                           nghttp2_data_source *source,
                           void *user_data) -> ssize_t {
    spdlog::debug(
        "{} Recive callback invoked! stream_id {}, length {}, data_flags {}",
        pthread_self(), //
        stream_id,      //
        length,         //
        *data_flags);

    auto *body = ((std::string *)source->ptr);

    auto *bodyData = body->data();
    auto bodyLen = std::min(body->size(), length);

    spdlog::debug("{} Copy {} bytes", pthread_self(), bodyLen);

    if (bodyLen > 0) {
      std::copy(bodyData, bodyData + bodyLen, buf);
      body->erase(0, bodyLen);

    } else {
      spdlog::debug("{} Set NGHTTP2_ERR_EOF flag", pthread_self());
      *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }

    return bodyLen;
  };

  nghttp2_data_provider provider{};
  provider.read_callback = reciveCallback;
  provider.source.ptr = (void *)&body;

  return AbstractSession::submit(url, methodStr, std::move(headers), id, weight,
                                 false, &provider);
}

std::tuple<AbstractSession::ErrorCode, int32_t>
HttpTlsSession::submit(Header &&headers, std::string_view url, Method method,
                       int32_t id, int32_t weight) noexcept {
  return this->submit(url,                           //
                      stringFromEnum(method).data(), //
                      std::move(headers),            //
                      id,                            //
                      weight                         //
  );
}

AbstractSession::Socket &HttpTlsSession::socket() {
  return d->service_.socket()->next_layer();
}

void HttpTlsSession::startConnect(AbstractSession::EndpointIt endpoint_it,
                                  AbstractSession::ErrorCode &ec) {

  if (!isStopped()) {
    ///\todo add error code assign
    //    ec.assign(static_cast<nghttp2_error>(NGHTTP2_INTERNAL_ERROR),
    //              NetworkErrorCategory{});
    return;
  }

  // Try to connect
  boost::asio::connect(socket(), endpoint_it, ec);

  if (ec.failed()) {
    spdlog::error("{} {}", pthread_self(), ec.message());
    return;
  }

  d->service_.socket()->handshake(boost::asio::ssl::stream_base::client, ec);

  if (ec.failed()) {
    spdlog::error("{} While handshaking: {}", pthread_self(), ec.message());
    stop();
    return;
  }

  if (!session_utils::tls_h2_negotiated(*d->service_.socket())) {
    return;
  }
}

bool HttpTlsSession::startResolve() {
  AbstractSession::ErrorCode ec;
  AbstractSession::startResolve(*d->service_.host(), *d->service_.port(), ec);
  return !ec.failed();
}

void HttpTlsSession::readSocket(AbstractSession::Handler handler) noexcept {
  try {
    AbstractSession::ErrorCode ec;

    d->service_.socket()->async_read_some(boost::asio::buffer(getReadBuffer()),
                                          handler);
    if (ec.failed())
      spdlog::error("{} {}", pthread_self(), ec.message());

  } catch (std::exception const &e) {
    spdlog::error("{} {}", pthread_self(), e.what());
    stop();
  }
}

void HttpTlsSession::writeSocket(AbstractSession::Handler h) noexcept {
  d->service_.socket()->async_write_some(
      boost::asio::buffer(getWriteBuffer(), getWriteBufferSize()), h);
}

void HttpTlsSession::shutdownSocket() noexcept {
  ///\todo Remove after GOAWAY will be realesed
  d->service_.nativeHandle()->stop();
}

constexpr std::string_view HttpTlsSession::stringFromEnum(Method method) {
  switch (method) {
  case Method::Get:
    return methodGet;

  case Method::Post:
    return methodPost;

  default:
    return "";
  }
}

AbstractSession::Method
HttpTlsSession::methodFromString(const std::string &method) {
  // Copy string and convert
  std::string uppercase = method;
  uppercase = utils::string::to_upper(uppercase);

  if (std::equal(methodGet.begin(), methodGet.end(), uppercase.begin()))
    return AbstractSession::Method::Get;

  if (std::equal(methodPost.begin(), methodPost.end(), uppercase.begin()))
    return AbstractSession::Method::Post;

  return AbstractSession::Method::Unknown;
}

} // namespace network
