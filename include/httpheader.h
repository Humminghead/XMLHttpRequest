#pragma once

#include <map>
#include <memory>
#include <string>

namespace network {

class HeaderValue;

// Header fields.  The header field name must be lower-cased.
class Header {
public:
  using Fields = std::multimap<std::string, HeaderValue>;
  using Iterator = Fields::iterator;

  Header();
  Header(Fields &&list);

  //  Header(const Header &) = default;
  //  Header &operator=(const Header &) = default;

  void emplace(std::string &&name, HeaderValue &&value) noexcept;
  void emplace(const std::string &name, const HeaderValue &value) noexcept;

  size_t size() const noexcept;

  Iterator begin();
  Iterator end();

  Iterator begin() const;
  Iterator end() const;

private:
  struct Impl;
  std::shared_ptr<Impl> d;
};
} // namespace::network
