#pragma once

#include <memory>
#include <string_view>
#include <vector>

namespace network {

class Header;

class Responce {
private:
  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;

public:
  using chunk_type = std::vector<uint8_t>;
  using data_type = std::vector<chunk_type>;

  Responce();
  ~Responce() = default;

  Responce(Responce &&) = default;
  Responce &operator=(Responce &&) = default;

  Responce(const Responce &) = delete;
  Responce &operator=(const Responce &) = delete;

  void statusCode(const int sc);
  int statusCode() const;

  void contentLength(const int64_t n);
  int64_t contentLength() const;

  Header &header();
  const Header &header() const;

  size_t headerBufferSize() const;
  void updateHeaderBufferSize(const size_t len);

  const data_type &data() const;
  void data(const uint8_t *data, const size_t len);

  const std::string text() const;
};

} // namespace network
