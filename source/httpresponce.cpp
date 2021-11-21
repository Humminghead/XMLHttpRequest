#include "httpresponce.h"
#include "httpheader.h"

#include <stdint.h>
#include <string.h>

namespace network {

struct Responce::Impl {
  size_t contentLength_{0};
  size_t headerBufferSize_{0};
  int statusCode_{-1};
  Header header_{};
  data_type data_{};
};

Responce::Responce() : d(new Responce::Impl(), [](auto p) { delete p; }) {}

void Responce::statusCode(const int sc) { d->statusCode_ = sc; }

int Responce::statusCode() const { return d->statusCode_; }

void Responce::contentLength(const int64_t n) { d->contentLength_ = n; }

int64_t Responce::contentLength() const { return d->contentLength_; }

Header &Responce::header() { return d->header_; }

const Header &Responce::header() const { return d->header_; }

size_t Responce::headerBufferSize() const { return d->headerBufferSize_; }

void Responce::updateHeaderBufferSize(const size_t len) {
  d->headerBufferSize_ = len;
}

const Responce::data_type &Responce::data() const { return d->data_; }

void Responce::data(const uint8_t *data, const size_t len) {
  auto chunk = chunk_type(len);
  std::copy(data, data + len, std::back_inserter(chunk));

  d->data_.push_back(std::move(chunk));
  d->contentLength_ += len;
}

} // namespace network
