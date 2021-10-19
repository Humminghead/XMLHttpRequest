#pragma once
#include <map>
#include <string>

namespace network {

struct HeaderValue {
  // Constructors
  HeaderValue() = default;

  HeaderValue(std::string &&val, bool sens = false)
      : value(std::move(val)), sensitive{sens} {}

  HeaderValue(const char *name, size_t size, bool sens = false)
      : value{name, size}, sensitive{sens} {}

  // header field value
  std::string value{};

  // true if the header field value is sensitive information, such as
  // authorization information or short length secret cookies.  If
  // true, those header fields are not indexed by HPACK (but still
  // huffman-encoded), which results in lesser compression.
  bool sensitive{false};
};

} // namespace::network
