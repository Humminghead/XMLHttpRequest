#pragma once

#include "types.h"

//#include "httptlssession.h"
#include "httpheader.h"
#include "xmlhttprequest.h"
//#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <future>
#include <memory>
#include <string>

namespace network {

class HttpTlsSession;

struct XMLHttpRequest::Impl {
  ///\todo del duplicate class members
  struct Elements {
    std::string addr{};
    std::string port{};
    std::string protocol{};
    std::string urn{};
  };

  int32_t fetch(const std::string &method, const std::string &uri);
  int32_t fetch(const std::string &method, const std::string &uri,
                const std::string &body);
  std::tuple<bool, Elements> proccessUri(const std::string &uri) noexcept;

  bool setupSession() noexcept;

  const std::string &certificate_ssl() const noexcept;

  void certificate_ssl(std::string &&cert) noexcept;

  void certificate_ssl(const std::string &cert) noexcept;

  void certificate_ssl(const std::string_view &cert) noexcept;

  std::shared_ptr<HttpTlsSession> session{nullptr};

  // Setup context and IO service
  std::shared_ptr<boost::asio::io_service> service;
  std::shared_ptr<boost::asio::ssl::context> contextSsl;

  bool isAsync{false};
  std::string addr{};
  std::string port{};
  std::string proto{};
  std::string urn{};
  std::string method{};
  std::string body{};
  std::string ssl_certificate{};
  Header header{};
  std::shared_ptr<Response> responce{nullptr};

  using future_t = std::future<std::size_t>;
  std::shared_ptr<future_t> future;

  ReadyState readyState{ReadyState::Unsent};

  // Is an number representing the number of milliseconds a request can take
  // before automatically being terminated.
  std::chrono::milliseconds timeout{0};
  std::shared_ptr<boost::asio::steady_timer> timer{nullptr};
  onTimeoutCallback timerCallback;
};
} // namespace network
