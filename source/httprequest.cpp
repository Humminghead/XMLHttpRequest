#include "httprequest.h"

#include "httpheader.h"
#include "httpheadervalue.h"
#include <algorithm>

namespace network {

struct Request::Impl {
  std::string scheme_;
  std::string host_;
  std::string method_;
  Header header_;
};

Request::Request() : d(new Request::Impl(), [](auto p) { delete p; }) {}

void Request::scheme(const char *value, size_t len) noexcept {
  (void)value;
  (void)len;

  ///\todo
  //  d->scheme_.assign(value, len);
}

void Request::host(const char *value, size_t len) noexcept {
  (void)value;
  (void)len;

  ///\todo
}

void Request::method(const char *value, size_t len) noexcept {
  (void)value;
  (void)len;

  ///\todo
}

std::string Request::scheme() const {
  if (d->scheme_.empty())
    d->scheme_ = this->find(":scheme");

  return d->scheme_;
}

std::string Request::host() const {
  if (d->host_.empty())
    d->host_ = this->find("host");

  return d->host_;
}

std::string Request::method() const {
  if (d->method_.empty())
    d->method_ = this->find(":method");

  return d->method_;
}

std::string Request::find(const std::string &name) const {

  auto it = std::find_if(std::begin(d->header_), std::end(d->header_),
                         [&](auto &header) { return header.first == name; });

  return (it == std::end(d->header_) ? "" : it->second.value);
}

Header &Request::header() { return d->header_; }

void Request::header(std::string name, HeaderValue &&value) noexcept {
  d->header_.emplace(std::move(name), std::move(value));
}

} // namespace::network
