#pragma once

#include <memory>
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

  void onData(const uint8_t *data, size_t len);

  Responce(const Responce &) = delete;
  Responce &operator=(const Responce &) = delete;

  void statusCode(int sc);
  int statusCode() const;

  void contentLength(int64_t n);
  int64_t contentLength() const;
  void contentLengthInc(int64_t n);

  virtual Header &header();
  virtual const Header &header() const;

  size_t headerBufferSize() const;
  void updateHeaderBufferSize(size_t len);

  const data_type &data() const;
};

} // namespace network
