#pragma once

#include <map>
#include <memory>
#include <string>

namespace network {

class HeaderValue;
class Header;

class Request {
private:
  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;

public:
  Request();

  void scheme(const char *value, size_t len) noexcept;
  void host(const char *value, size_t len) noexcept;
  void method(const char *value, size_t len) noexcept;

  std::string scheme() const;
  std::string host() const;
  std::string method() const;

  std::string find(const std::string &) const;

  Header &header();
  void header(std::string name, HeaderValue &&value) noexcept;

  //  void onClose(uint32_t errorCode);
};

} // namespace::network
