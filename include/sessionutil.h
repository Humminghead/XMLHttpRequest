#pragma once

#include <arpa/inet.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <nghttp2/nghttp2.h>
#include <string>
#include <string_view>

///\todo fix include path
#include "../third-party/url-parser/url_parser.h"

namespace network::session_utils {

constexpr auto NGHTTP2_H2 = std::string_view("h2");

// The additional HTTP/2 protocol ALPN protocol identifier we also
// supports for our applications to make smooth migration into final
// h2 ALPN ID.
constexpr auto NGHTTP2_H2_16 = std::string_view("h2-16");
constexpr auto NGHTTP2_H2_14 = std::string_view("h2-14");

static void copy_url_component(std::string &dest, const http_parser_url *u,
                               int field, const char *url) {
  if (u->field_set & (1 << field)) {
    dest.assign(url + u->field_data[field].off, u->field_data[field].len);
  }
}

static bool ipv6_numeric_addr(const char *host) {
  uint8_t dst[16];
  return inet_pton(AF_INET6, host, dst) == 1;
}

template <typename T> static std::string utos(T n) {
  std::string res;
  if (n == 0) {
    res = "0";
    return res;
  }
  size_t nlen = 0;
  for (auto t = n; t; t /= 10, ++nlen)
    ;
  res.resize(nlen);
  for (; n; n /= 10) {
    res[--nlen] = (n % 10) + '0';
  }
  return res;
}

// Create nghttp2_nv from string literal |name| and std::string
// |value|.
template <size_t N>
static nghttp2_nv make_nv_ls(const char (&name)[N], const std::string &value) {
  return {(uint8_t *)name, (uint8_t *)value.c_str(), N - 1, value.size(),
          NGHTTP2_NV_FLAG_NO_COPY_NAME};
}
static nghttp2_nv make_nv_internal(const std::string &name,
                                   const std::string &value, bool no_index,
                                   uint8_t nv_flags) {
  uint8_t flags;

  flags =
      nv_flags | (no_index ? NGHTTP2_NV_FLAG_NO_INDEX : NGHTTP2_NV_FLAG_NONE);

  return {(uint8_t *)name.c_str(), (uint8_t *)value.c_str(), name.size(),
          value.size(), flags};
}
static nghttp2_nv make_nv(const std::string &name, const std::string &value,
                          bool no_index) {
  return make_nv_internal(name, value, no_index, NGHTTP2_NV_FLAG_NONE);
}
inline bool is_digit(const char c) { return '0' <= c && c <= '9'; }

inline bool is_hex_digit(const char c) {
  return is_digit(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

static uint32_t hex_to_uint(char c) {
  if (c <= '9') {
    return c - '0';
  }
  if (c <= 'Z') {
    return c - 'A' + 10;
  }
  if (c <= 'z') {
    return c - 'a' + 10;
  }
  return 256;
}

template <typename InputIt>
static std::string percent_decode(InputIt first, InputIt last) {
  std::string result;
  result.resize(last - first);
  auto p = std::begin(result);
  for (; first != last; ++first) {
    if (*first != '%') {
      *p++ = *first;
      continue;
    }

    if (first + 1 != last && first + 2 != last && is_hex_digit(*(first + 1)) &&
        is_hex_digit(*(first + 2))) {
      *p++ = (hex_to_uint(*(first + 1)) << 4) + hex_to_uint(*(first + 2));
      first += 2;
      continue;
    }

    *p++ = *first;
  }
  result.resize(p - std::begin(result));
  return result;
}

static bool check_h2_is_selected(const std::string &proto) {
  return NGHTTP2_H2 == proto || NGHTTP2_H2_16 == proto ||
         NGHTTP2_H2_14 == proto;
}

static bool tls_h2_negotiated(
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &socket) {
  auto ssl = socket.native_handle();

  const unsigned char *next_proto = nullptr;
  unsigned int next_proto_len = 0;

#ifndef OPENSSL_NO_NEXTPROTONEG
  SSL_get0_next_proto_negotiated(ssl, &next_proto, &next_proto_len);
#endif // !OPENSSL_NO_NEXTPROTONEG
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
  if (next_proto == nullptr) {
    SSL_get0_alpn_selected(ssl, &next_proto, &next_proto_len);
  }
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L

  if (next_proto == nullptr) {
    return false;
  }
  std::string sv((char *)next_proto, next_proto_len);
  return session_utils::check_h2_is_selected(sv);
}

static std::pair<int64_t, size_t> parse_uint_digits(const void *ss,
                                                    size_t len) {
  const uint8_t *s = static_cast<const uint8_t *>(ss);
  int64_t n = 0;
  size_t i;
  if (len == 0) {
    return {-1, 0};
  }
  constexpr int64_t max = std::numeric_limits<int64_t>::max();
  for (i = 0; i < len; ++i) {
    if ('0' <= s[i] && s[i] <= '9') {
      if (n > max / 10) {
        return {-1, 0};
      }
      n *= 10;
      if (n > max - (s[i] - '0')) {
        return {-1, 0};
      }
      n += s[i] - '0';
      continue;
    }
    break;
  }
  if (i == 0) {
    return {-1, 0};
  }
  return {n, i};
}

static int64_t parse_uint(const uint8_t *s, size_t len) {
  int64_t n;
  size_t i;
  std::tie(n, i) = parse_uint_digits(s, len);
  if (n == -1 || i != len) {
    return -1;
  }
  return n;
}

} // namespace network::session_utils
//----------------------------------------------
