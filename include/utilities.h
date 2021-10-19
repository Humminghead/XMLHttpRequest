#pragma once

#include <boost/asio/ssl.hpp>
#include <boost/regex.hpp>
#include <functional>

namespace utils::string {

/*!
 * \short Replaces characters in the input string with characters passed to the
 * function by the condition of the regular expression
 * \param Regular expression
 * \param Souce string
 * \param Part of string what should be replaced
 */
[[maybe_unused]] static std::string
replace(const char *regexp, const std::string &what, const std::string &than) {
  boost::regex re{regexp};
  std::string s{what};

  return boost::regex_replace(s, re, than);
}
/*!
 * \brief split
 * \param in
 * \param splitter
 * \param pos
 */
[[maybe_unused]] static auto split(const std::string &in, const char splitter,
                                   const size_t pos = 0) {
  if (in.empty())
    return std::make_tuple(std::string{}, std::string{});

  if (auto p = in.find_first_of(splitter, pos); p) {
    return std::make_tuple(
        in.substr(pos, p),
        in.substr(p + 1, in.length())); // p + 1 because we exclude splitter
  }

  return std::make_tuple(in, std::string{});
}
/*!
 * \brief Compare two strings
 */
template <typename InputIt1, typename InputIt2>
bool streq(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
  if (std::distance(first1, last1) != std::distance(first2, last2)) {
    return false;
  }
  return std::equal(first1, last1, first2);
}

template <typename T, typename S> bool streq(const T &a, const S &b) {
  return streq(a.begin(), a.end(), b.begin(), b.end());
}

template <typename CharT, typename InputIt, size_t N>
bool streq_l(const CharT (&a)[N], InputIt b, size_t blen) {
  return streq(a, a + (N - 1), b, b + blen);
}

template <typename CharT, size_t N, typename T>
bool streq_l(const CharT (&a)[N], const T &b) {
  return streq(a, a + (N - 1), b.begin(), b.end());
}

[[maybe_unused]] static std::string to_upper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); } // correct
  );
  return s;
};

} // namespace utils::string

namespace utils::commands {

// inspired by <http://blog.korfuri.fr/post/go-defer-in-cpp/>, but our
// template can take functions returning other than void.
template <typename F, typename... T> struct Defer {
  Defer(F &&f, T &&...t)
      : f(std::bind(std::forward<F>(f), std::forward<T>(t)...)) {}
  Defer(Defer &&o) noexcept : f(std::move(o.f)) {}
  ~Defer() { f(); }

  using ResultType = typename std::result_of<typename std::decay<F>::type(
      typename std::decay<T>::type...)>::type;
  std::function<ResultType()> f;
};

template <typename F, typename... T> auto defer(F &&f, T &&...t) {
  return Defer<F, T...>(std::forward<F>(f), std::forward<T>(t)...);
}
} // namespace map::utils::commands

namespace utils::ssl {

namespace {
constexpr auto NGHTTP2_H2_ALPN = std::string_view("\x2h2");
constexpr auto NGHTTP2_H2 = std::string_view("h2");

// The additional HTTP/2 protocol ALPN protocol identifier we also
// supports for our applications to make smooth migration into final
// h2 ALPN ID.
constexpr auto NGHTTP2_H2_16_ALPN = std::string_view("\x5h2-16");
constexpr auto NGHTTP2_H2_16 = std::string_view("h2-16");

constexpr auto NGHTTP2_H2_14_ALPN = std::string_view("\x5h2-14");
constexpr auto NGHTTP2_H2_14 = std::string_view("h2-14");

constexpr auto NGHTTP2_H1_1_ALPN = std::string_view("\x8http/1.1");
constexpr auto NGHTTP2_H1_1 = std::string_view("http/1.1");

std::vector<unsigned char> getDefaultAlpn() {
  auto res = std::vector<unsigned char>(NGHTTP2_H2_ALPN.size() +
                                        NGHTTP2_H2_16_ALPN.size() +
                                        NGHTTP2_H2_14_ALPN.size());
  auto p = std::begin(res);

  p = std::copy_n(std::begin(NGHTTP2_H2_ALPN), NGHTTP2_H2_ALPN.size(), p);
  p = std::copy_n(std::begin(NGHTTP2_H2_16_ALPN), NGHTTP2_H2_16_ALPN.size(), p);
  p = std::copy_n(std::begin(NGHTTP2_H2_14_ALPN), NGHTTP2_H2_14_ALPN.size(), p);

  return res;
}

bool selectProto(const unsigned char **out, unsigned char *outlen,
                 const unsigned char *in, unsigned int inlen,
                 const std::string_view &key) {
  for (auto p = in, end = in + inlen; p + key.size() <= end; p += *p + 1) {
    if (std::equal(std::begin(key), std::end(key), p)) {
      *out = p + 1;
      *outlen = *p;
      return true;
    }
  }
  return false;
}
} // namespace

///\brief Loads certificate to it context
[[maybe_unused]] static auto loadCertificate(
    [](const std::string &cert, boost::asio::ssl::context &context,
       boost::system::error_code &ec) {
      ec.clear();
      if (!cert.empty())
        context.add_certificate_authority(
            boost::asio::buffer(
                reinterpret_cast<void *>(const_cast<char *>(cert.c_str())),
                cert.size()),
            ec);
      return ec;
    });

static bool selectH2(const unsigned char **out, unsigned char *outlen,
                     const unsigned char *in, unsigned int inlen) {
  return selectProto(out, outlen, in, inlen, NGHTTP2_H2_ALPN) ||
         selectProto(out, outlen, in, inlen, NGHTTP2_H2_16_ALPN) ||
         selectProto(out, outlen, in, inlen, NGHTTP2_H2_14_ALPN);
}

static int clientSelectNextProtoCb(SSL *ssl, unsigned char **out,
                                   unsigned char *outlen,
                                   const unsigned char *in, unsigned int inlen,
                                   void *arg) {
  (void)ssl;
  (void)arg;

  if (!selectH2(const_cast<const unsigned char **>(out), outlen, in, inlen)) {
    return SSL_TLSEXT_ERR_NOACK;
  }
  return SSL_TLSEXT_ERR_OK;
}

[[maybe_unused]] static auto configureTlsContext(
    [](boost::system::error_code &ec, boost::asio::ssl::context &context) {
      ec.clear();

      auto ctx = context.native_handle();

#ifndef OPENSSL_NO_NEXTPROTONEG
      SSL_CTX_set_next_proto_select_cb(ctx, clientSelectNextProtoCb, nullptr);
#endif // !OPENSSL_NO_NEXTPROTONEG

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
      auto proto_list = getDefaultAlpn();

      SSL_CTX_set_alpn_protos(ctx, proto_list.data(), proto_list.size());
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L

      return ec;
    });

[[maybe_unused]] static auto setCipherList(std::string_view list,
                                           boost::asio::ssl::context &context) {
  return SSL_CTX_set_cipher_list(context.native_handle(), list.data());
}

} // namespace utils::ssl
