#include "sessionstream.h"

#include "abstractsession.h"
#include "httprequest.h"
#include "httpresponce.h"
#include <nghttp2/nghttp2.h>

namespace network {
struct Stream::Impl {
  std::shared_ptr<Request> rq{nullptr};
  std::shared_ptr<Response> rp{nullptr};
  uint32_t stream_id_{0};
  nghttp2_stream *stream_{nullptr};
  uint32_t statusCode{0};
  Stream::State state{State::Idle};
};

Stream::Stream() : d{new Impl(), [](auto *p) { delete p; }} {

  d->rp = std::make_shared<Response>();
  d->rq = std::make_shared<Request>();
}

Stream::Stream(const int id, nghttp2_stream *strm)
    : d{new Impl(), [](auto *p) { delete p; }} {
  this->id(static_cast<decltype(Impl::stream_id_)>(id));
  this->stream(strm);

  d->rp = std::make_shared<Response>();
  d->rq = std::make_shared<Request>();
}

Stream::~Stream() { state(State::Closed); }

void Stream::id(int32_t stream_id) { d->stream_id_ = stream_id; }

int32_t Stream::id() const { return d->stream_id_; }

nghttp2_stream *Stream::stream() const { return d->stream_; }

void Stream::stream(nghttp2_stream *strm) { d->stream_ = strm; }

std::shared_ptr<Request> Stream::request() { return d->rq; }

std::shared_ptr<Response> Stream::response() { return d->rp; }

uint32_t Stream::status() const { return d->statusCode; }

void Stream::status(uint32_t code) const { d->statusCode = code; }

bool Stream::expectFinalResponse() const { return status() / 100 == 1; }

Stream::State Stream::state() const { return d->state; }

void Stream::state(Stream::State s) const { d->state = s; }

} // namespace::network
