#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

struct nghttp2_stream;

namespace network {
class Request;
class Responce;
class AbstractSession;

class Stream {
public:
  // http://book.itep.ru/4/45/http2.htm
  enum class State {
    Idle,
    ReservedLocal,
    ReservedRemote,
    Open,
    HalfClosedLocal,
    HalfClosedRemote,
    Closed
  };

  Stream();
  Stream(const int id, nghttp2_stream *strm);
  ~Stream();

  Stream(const Stream &) = delete;
  Stream &operator=(const Stream &) = delete;

  void id(int32_t stream_id);
  int32_t id() const;

  nghttp2_stream *stream() const;
  void stream(nghttp2_stream *strm);

  std::shared_ptr<Request> request();
  std::shared_ptr<Responce> response();

  uint32_t status() const;
  void status(uint32_t code) const;

  bool expectFinalResponse() const;

  State state() const;
  void state(State s) const;

private:
  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;
};

using StreamPtr = std::shared_ptr<Stream>;
using Streams = std::unordered_map<size_t, StreamPtr>;

} // namespace::network
