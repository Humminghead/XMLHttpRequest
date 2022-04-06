#include "httpresponce.h"
#include "httpheader.h"
#include "httpheadervalue.h"

#include <stdint.h>
#include <string.h>

#include <algorithm>

namespace network {

struct Response::Impl {
  size_t contentLength_{0};
  size_t headerBufferSize_{0};
  int statusCode_{-1};
  Header header_{};
  data_type data_{};
};

Response::Response() : d(new Response::Impl(), [](auto p) { delete p; }) {}

void Response::statusCode(const int sc) { d->statusCode_ = sc; }

int Response::statusCode() const { return d->statusCode_; }
///\todo FIX  contentLength in GET
void Response::contentLength(const int64_t n) { d->contentLength_ = n; }

int64_t Response::contentLength() const { return d->contentLength_; }

Header &Response::header() { return d->header_; }

const Header &Response::header() const { return d->header_; }

size_t Response::headerBufferSize() const { return d->headerBufferSize_; }

void Response::updateHeaderBufferSize(const size_t len) {
  d->headerBufferSize_ = len;
}

const Response::data_type &Response::data() const { return d->data_; }

void Response::data(const uint8_t *&data, const size_t len) {
  auto chunk = chunk_type{};
  chunk.reserve(len);

  d->contentLength_ += len;

  std::copy(data, data + len, std::back_inserter(chunk));
  d->data_.push_back(std::move(chunk));
}

const std::string Response::text() const {
  auto it = std::find_if(d->header_.begin(), d->header_.end(), [](auto &pair) {
    return (pair.second.view().find("charset=") != std::string_view::npos);
  });

  if (it != std::end(d->header_)) {

    std::string text;
    text.reserve(contentLength());

    for (auto &chunk : d->data_)
      text.append((char *)chunk.data(), chunk.size());

    return text;
  }

  return {};
}
} // namespace network
