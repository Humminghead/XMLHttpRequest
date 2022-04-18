#pragma once

#include <memory>
#include <string_view>
#include <vector>

namespace network {

class Header;

class Response {
private:
  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;

public:
  using chunk_type = std::vector<uint8_t>;
  using data_type = std::vector<chunk_type>;

  Response();
  ~Response() = default;

  Response(Response &&) = default;
  Response &operator=(Response &&) = default;

  Response(const Response &) = delete;
  Response &operator=(const Response &) = delete;

  void statusCode(const int sc);
  int statusCode() const;

  void contentLength(const int64_t n);
  int64_t contentLength() const;

  Header &header();
  const Header &header() const;

  size_t headerBufferSize() const;
  void updateHeaderBufferSize(const size_t len);

  const data_type &data() const;
  void data(const uint8_t *&data, const size_t len);

  const std::string text() const;

  const std::string_view type() const;
  bool type(const std::string_view& type);
};

} // namespace network
