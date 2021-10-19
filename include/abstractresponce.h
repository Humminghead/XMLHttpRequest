#pragma once
#include "httpheader.h"
#include "httpheadervalue.h"

#include <memory>
#include <nghttp2/nghttp2.h>

namespace network {

template <class T>
class AbstractResponse : std::enable_shared_from_this<AbstractResponse<T>> {
public:
  using value_type = T;

  AbstractResponse() {}
  virtual ~AbstractResponse() {}

  AbstractResponse(const AbstractResponse &) = delete;
  AbstractResponse &operator=(const AbstractResponse &) = delete;

  virtual void onData(const uint8_t *data, size_t len) = 0;

  //  void call_on_data(const uint8_t *data, std::size_t len){}

  void statusCode(int sc) { statusCode_ = sc; }
  int statusCode() const { return statusCode_; };

  void contentLength(int64_t n) { contentLength_ = n; }
  int64_t contentLength() const { return contentLength_; }
  void contentLengthInc(int64_t n) { contentLength_ += n; }

  virtual Header &header() { return header_; }
  virtual const Header &header() const { return header_; }

  size_t headerBufferSize() const { return headerBufferSize_; }
  void updateHeaderBufferSize(size_t len) { headerBufferSize_ = len; }

  auto getPointer() { return this->shared_from_this(); }

private:
  //  data_cb data_cb_;

  size_t contentLength_;
  size_t headerBufferSize_;
  int statusCode_;
  Header header_;
};
} // namespace network
