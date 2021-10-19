#pragma once

#include "abstractsession.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ssl/rfc2818_verification.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <string>

namespace network {

class NetService {
public:
  NetService(boost::asio::io_service &io_service,
             boost::asio::ssl::context &tlsCtx, const std::string &hst,
             const std::string &srv)
      : service_{io_service}, host_{hst}, port_{srv}, sock_{service_, tlsCtx} {
    sock_.set_verify_callback(boost::asio::ssl::rfc2818_verification(host_));
  }
  auto socket() noexcept { return &sock_; }

  auto host() noexcept { return &host_; }
  auto port() noexcept { return &port_; }
  auto nativeHandle() noexcept { return &service_; }

private:
  boost::asio::io_service &service_;
  std::string host_;
  std::string port_;
  boost::asio::ssl::stream<AbstractSession::Socket> sock_;
};
} // namespace::network
