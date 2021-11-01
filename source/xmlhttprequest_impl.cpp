#include "xmlhttprequest_impl.h"

#include <spdlog/spdlog.h>

#include <boost/regex.hpp>
#include <memory>
#include <utility>

#include "defaults.h"
#include "httpheader.h"
#include "httpheadervalue.h"
#include "httptlssession.h"
#include "utilities.h"

#ifdef DEBUG_SSL
#include <fstream>
#endif

namespace network {
[[maybe_unused]] static constexpr std::string_view regexGroupProtocol{
    R"((^[https]{4,5}))"};
[[maybe_unused]] static constexpr std::string_view regexGroupHostPort{
    R"(([0-9a-z\.]{3,}))"};
[[maybe_unused]] static constexpr std::string_view regexAddressOrHostPort{
    R"(([0-9a-z\.:]{3,}))"};
[[maybe_unused]] static constexpr std::string_view regexDigitChains{
    R"(([0-9]+))"};
[[maybe_unused]] static constexpr std::string_view regexGroupIpPort{
    R"(((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]):[0-9]+))"};
[[maybe_unused]] static constexpr std::string_view addressTail{
    R"((\/{0,1}.{0,}))"};
[[maybe_unused]] static constexpr std::string_view checkHostPort{
    R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9]):[0-9]+$)"};

[[maybe_unused]] static std::string baseRegexp{
    std::string{regexGroupProtocol} +   //
    std::string{R"(:\/\/)"} +           //
    std::string{regexAddressOrHostPort} //
};

[[maybe_unused]] static std::string protoIpPortUrlRegex{
    std::string{baseRegexp} + //
    std::string{addressTail}  //
};

int32_t XMLHttpRequest::Impl::fetch(const std::string &method,
                                    const std::string &uri)

{
  spdlog::trace("{} XMLHttpRequest::Impl::fetch({})", pthread_self(), uri);

  // Get return value type
  //    using submit_ret_t = decltype(
  //        std::declval<HttpTlsSession>().submit(Header{}, "",
  //        HttpTlsSession::methodFromString("")));

  // Convert method from string
  auto methodValue = HttpTlsSession::methodFromString(method);

  if (uri.empty() || !session ||
      methodValue == AbstractSession::Method::Unknown)
    return -1;

  auto [ec, id] = session->submit(std::move(header), uri, methodValue);

  if (ec.failed()) {
    spdlog::error("{} {}", pthread_self(), ec.message());
  }

  return id;
}

int32_t XMLHttpRequest::Impl::fetch(const std::string &method,
                                    const std::string &uri,
                                    const std::string &body) {
  ///\todo
  ///
  auto methodValue = HttpTlsSession::methodFromString(method);

  if (uri.empty() || !session ||
      methodValue == AbstractSession::Method::Unknown)
    return -1;

  auto [ec, id] = session->submit(std::move(header), uri, methodValue, body);

  if (ec.failed()) {
    spdlog::error("{} {}", pthread_self(), ec.message());
  }

  return id;
}

bool XMLHttpRequest::Impl::setupSession() noexcept {
  boost::system::error_code ec;

  this->service.reset();
  this->contextSsl.reset();

  this->service = std::make_shared<boost::asio::io_service>();
  this->contextSsl = std::make_shared<boost::asio::ssl::context>(
      boost::asio::ssl::context::tlsv13_client);

  if (!ssl_certificate.empty() &&
      utils::ssl::loadCertificate(ssl_certificate.data(), *contextSsl, ec)
          .failed()) {
    spdlog::error("{} {}", pthread_self(), "Certificate load failed! ");
    return false;
  }

  if (utils::ssl::configureTlsContext(ec, *contextSsl).failed()) {
    spdlog::error("{} {}", pthread_self(), ec.message());
    return false;
  }

  if (!utils::ssl::setCipherList(defaults::default_cipher_list, *contextSsl)) {
    spdlog::debug("{} {}", pthread_self(), "SSL cipher list not loaded!");
  };

#ifdef DEBUG_SSL
  SSL_CTX_set_keylog_callback(
      contextSsl->native_handle(), [](const SSL *ssl, const char *line) {
        (void)ssl;

        std::ofstream ostrm(R"(/tmp/ssl-key-log.txt)",
                            std::ios::binary | std::ios::app);
        std::string str{line};
        ostrm << str << std::endl;
        ostrm.write(str.c_str(), str.size()); // binary output
      });
#endif

  session.reset();

  if (session =
          std::make_shared<HttpTlsSession>(*service, *contextSsl, addr, port);
      session != nullptr)
    return true;

  return false;
}

const std::string &XMLHttpRequest::Impl::certificate_ssl() const noexcept {
  return ssl_certificate;
}

void XMLHttpRequest::Impl::certificate_ssl(std::string &&cert) noexcept {
  ssl_certificate = std::move(cert);
}

void XMLHttpRequest::Impl::certificate_ssl(const std::string &cert) noexcept {
  ssl_certificate = cert;
}

void XMLHttpRequest::Impl::certificate_ssl(
    const std::string_view &cert) noexcept {
  ssl_certificate = cert;
}

std::tuple<bool, XMLHttpRequest::Impl::Elements>
XMLHttpRequest::Impl::proccessUri(const std::string &uri) noexcept {
  if (uri.empty())
    return {};
  std::cerr << protoIpPortUrlRegex << "\n";
  if (boost::smatch what;
      boost::regex_search(uri, what, boost::regex{protoIpPortUrlRegex})) {
    //'what' variable is it const refence (usage of move semantics is it
    // impossible)
    //    for (int i = 0; i < 6;)
    //      std::cerr << what[i++] << std::endl;

    std::string urlProtocol{what[1]};
    std::string urlAddr{what[2]};
    std::string urlTail{what[3]};
    std::string urlPort{};

    if (urlProtocol.empty() || urlAddr.empty() /* || urlTail.empty()*/)
      return {};

    if (auto match = boost::regex_match(
            urlAddr, what, boost::regex(std::string{checkHostPort}));
        match) {
      if (auto pos = urlAddr.find_last_of(R"(:)"); pos != std::string::npos) {
        urlPort = urlAddr.substr(++pos, urlAddr.size());
        urlAddr.erase(--pos, urlAddr.size());
      }
    }

    // If port not found
    urlPort =
        urlPort.empty() ? (urlProtocol == "https" ? "443" : "80") : urlPort;

    return std::make_tuple(true, Elements{
                                     std::move(urlAddr),     //
                                     std::move(urlPort),     //
                                     std::move(urlProtocol), //
                                     std::move(urlTail)      //
                                 });
  }
  return {};
}

} // namespace network
