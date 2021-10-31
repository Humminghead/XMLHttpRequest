#include "sessioncallbacks.h"

#include <algorithm>
#include <ctype.h>
#include <spdlog/spdlog.h>

#include "abstractsession.h"
#include "http2.h"
#include "httpheadervalue.h"
#include "httprequest.h"
#include "httpresponce.h"
#include "sessionstream.h"
#include "sessionutil.h"
#include "xmlhttprequeststate.h"

namespace network::callbacks {
auto createAndAddStream([](std::shared_ptr<AbstractSession> s,
                           const nghttp2_frame *frame) {
  auto strm = std::make_shared<Stream>(
      frame->hd.stream_id,
      nghttp2_session_find_stream(s->rawSession(), frame->hd.stream_id));

  if (s->addStream(strm))
    return strm;

  return decltype(strm){};
});

int onBeginHeadersCallback(nghttp2_session *session, const nghttp2_frame *frame,
                           void *user_data) {
  (void)session;

  spdlog::debug("{} onBeginHeadersCallback", pthread_self());

  auto holder = *(static_cast<AbstractSession::SessionHolder *>(user_data));
  auto s = holder();

  if (frame->hd.type != NGHTTP2_PUSH_PROMISE) {
    createAndAddStream(s, frame);
  } else {
    s->addStream(s->createPushStream(frame->push_promise.promised_stream_id));
  }

  return 0;
}

int onHeaderCallback(nghttp2_session *session, const nghttp2_frame *frame,
                     const uint8_t *name, size_t namelen, const uint8_t *value,
                     size_t valuelen, uint8_t flags, void *user_data) {
  spdlog::trace("{} onHeaderCallback", pthread_self());

  (void)session;
  (void)flags;

  auto holder = *(static_cast<AbstractSession::SessionHolder *>(user_data));
  auto s = holder();

  switch (frame->hd.type) {
  case NGHTTP2_HEADERS: {
    if (auto cb = s->getOnStateChangeCallback(); cb)
      cb(static_cast<uint8_t>(ReadyState::HeadersReceived));

    auto strm = s->findStream(frame->hd.stream_id);
    if (!strm) {
      spdlog::trace("{} NGHTTP2_HEADERS: stream {} not found ", pthread_self(),
                    frame->hd.stream_id);
      return 0;
    }

    // ignore trailers
    if (frame->headers.cat == NGHTTP2_HCAT_HEADERS /*&&
        !strm->expect_final_response()*/) {
      spdlog::trace("{} NGHTTP2_HCAT_HEADERS", pthread_self());
      return 0;
    }

    auto res = strm->response();

    {
      auto token = http2::lookup_token(name, namelen);

      spdlog::debug("{} Token: {} Value: {}", pthread_self(),
                    std::string_view((const char *)name, namelen),
                    std::string_view((const char *)value, valuelen));

      if (token == http2::HD__STATUS) {
        res->statusCode(session_utils::parse_uint(value, valuelen));
      } else if (token == http2::HD_CONTENT_LENGTH) {
        res->contentLength(session_utils::parse_uint(value, valuelen));
      } else if (token == http2::HD_CONTENT_TYPE) {
        // Set overriden mime type
        if (auto &type = s->getMimeOverridenType(); !type.empty()) {
          if (auto commaPos = type.find_first_of(';');
              commaPos != std::string::npos && commaPos + 1 <= type.size()) {
            auto [name, value] =
                std::tuple{type.substr(0, commaPos),
                           type.substr(commaPos + 1, type.size())};

            // Clear all spaces
            value.erase(std::remove_if(value.begin(), value.end(),
                                       [](const auto c) { //
                                         return std::isspace(c);
                                       }),
                        value.end());

            spdlog::debug("{} Set overriden mime type: Token: {} Value: {}",
                          pthread_self(), name, value);

            res->header().emplace(
                std::move(name),
                HeaderValue{std::move(value),
                            (flags & NGHTTP2_NV_FLAG_NO_INDEX) != 0});
          }
          break;
        }
      }

      // Save header field
      res->header().emplace(
          std::string(name, name + namelen),
          HeaderValue{std::string(value, value + valuelen),
                      (flags & NGHTTP2_NV_FLAG_NO_INDEX) != 0});
      break;
    }
  }

  case NGHTTP2_PUSH_PROMISE: {
    spdlog::trace("{} NGHTTP2_PUSH_PROMISE", pthread_self());

    auto strm = s->findStream(frame->push_promise.promised_stream_id);

    if (!strm) {
      return 0;
    }

    if (strm->state() == Stream::State::Idle) {
      spdlog::debug("{} Stream{} State::Idle", pthread_self(), strm->id());
      strm->state(Stream::State::ReservedRemote);
    }

    auto req = strm->request();
    auto token = http2::lookup_token(name, namelen);
    spdlog::debug(
        "{} Token: {} Value: {}", pthread_self(),
        std::string_view(reinterpret_cast<const char *>(name), namelen),
        std::string_view(reinterpret_cast<const char *>(value), valuelen));

    {
      switch (token) {
      case http2::HD__METHOD:
        req->method(reinterpret_cast<const char *>(value), valuelen);
        break;
      case http2::HD__SCHEME:
        req->scheme(reinterpret_cast<const char *>(value), valuelen);
        break;
      case http2::HD__PATH:
        //      split_path(uri, value, value + valuelen);
        break;
      case http2::HD__AUTHORITY:
        req->host(reinterpret_cast<const char *>(value), valuelen);
        break;
      case http2::HD_HOST:
        if (req->host().empty()) {
          req->host(reinterpret_cast<const char *>(value), valuelen);
        }
        // fall through
      default:
        ///\todo
        /*
            if (req.header_buffer_size() + namelen + valuelen > 64_k) {
              nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                        frame->hd.stream_id,
           NGHTTP2_INTERNAL_ERROR); strm->state(Stream::State::Closed); break;
            }
            //            req.update_header_buffer_size(namelen + valuelen);
        */
        req->header(std::string(name, name + namelen),
                    HeaderValue{std::string(value, value + valuelen),
                                (flags & NGHTTP2_NV_FLAG_NO_INDEX) != 0});
        break;
      }
    }
  }
  }

  return 0;
}

int onFrameRecvCallback(nghttp2_session *session, const nghttp2_frame *frame,
                        void *user_data) {
  (void)session;

  spdlog::trace("{} onFrameRecvCallback()", pthread_self());

  auto holder = *(static_cast<AbstractSession::SessionHolder *>(user_data));
  auto s = holder();

  if (auto strm = s->findStream(frame->hd.stream_id); strm) {
    spdlog::trace("{} Stream {} found! frame->hd.type = {} ", pthread_self(),
                  frame->hd.stream_id, frame->hd.type);

    switch (frame->hd.type) {
    case NGHTTP2_DATA: {
      spdlog::trace("{} NGHTTP2_DATA: stream {}", pthread_self(),
                    frame->hd.stream_id);

      if (!strm) {
        spdlog::trace("{} NGHTTP2_DATA: stream {} not found ", pthread_self(),
                      frame->hd.stream_id);
        break;
      }

      if (strm->state() == Stream::State::Idle) {
        spdlog::error("{} Stream{} in state idle! Protocol error!",
                      pthread_self(), strm->id());
        return 0;
      }

      if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        spdlog::debug("{} Stream{} State::HalfClosedRemote", pthread_self(),
                      strm->id());
        strm->state(Stream::State::HalfClosedRemote);
      }
    } break;
    case NGHTTP2_HEADERS: {
      spdlog::trace("{} NGHTTP2_HEADERS: stream {}", pthread_self(),
                    frame->hd.stream_id);
      if (!strm) {
        break;
      }

      if (strm->state() == Stream::State::Idle) {
        spdlog::debug("{} Stream{} State::Open", pthread_self(), strm->id());
        strm->state(Stream::State::Open);
      }

      if (frame->headers.cat == NGHTTP2_HCAT_HEADERS &&
          !strm->expectFinalResponse()) {
        spdlog::debug("{} Stream{} NGHTTP2_HCAT_HEADERS", pthread_self(),
                      strm->id());
        return 0;
      }

      if (strm->expectFinalResponse()) {
        spdlog::debug("{} Stream{} wait for final response", pthread_self(),
                      strm->id());
        return 0;
      }

      if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        spdlog::debug("{} Stream{} State::HalfClosedRemote", pthread_self(),
                      strm->id());
        strm->state(Stream::State::HalfClosedRemote);
      }

    } break;
    case NGHTTP2_SETTINGS: {
      spdlog::trace("{} NGHTTP2_SETTINGS: stream {}", pthread_self(),
                    frame->hd.stream_id);

      if (strm->state() == Stream::State::Idle) {
        spdlog::error("{} Stream{} in state idle! Protocol error!",
                      pthread_self(), strm->id());
        return 0;
      }
    } break;
    case NGHTTP2_WINDOW_UPDATE: {
      spdlog::trace("{} NGHTTP2_WINDOW_UPDATE: stream {}", pthread_self(),
                    frame->hd.stream_id);

      if (strm->state() == Stream::State::Idle) {
        spdlog::error("{} Stream{} in state idle! Protocol error!",
                      pthread_self(), strm->id());
        return 0;
      }
    } break;
    case NGHTTP2_PUSH_PROMISE: {
      spdlog::trace("{} NGHTTP2_PUSH_PROMISE: stream {}", pthread_self(),
                    frame->hd.stream_id);

      if (strm->state() == Stream::State::Idle) {
        spdlog::error("{} Stream{} in state idle! Protocol error!",
                      pthread_self(), strm->id());
        return 0;
      }

      spdlog::debug("{} Stream{} State::ReservedRemote", pthread_self(),
                    strm->id());
      strm->state(Stream::State::ReservedRemote);
    } break;
    case NGHTTP2_GOAWAY: {
      spdlog::debug("{} Stream{} GOAWAY. Switch to State::Closed",
                    pthread_self(), frame->hd.stream_id);
      strm->state(Stream::State::Closed);
    } break;
    }
  } else {
    spdlog::trace("{} Stream {} not found ", pthread_self(),
                  frame->hd.stream_id);
  }

  return 0;
}

int onDataChunkRecvCallback(nghttp2_session *session, uint8_t flags,
                            int32_t stream_id, const uint8_t *data, size_t len,
                            void *user_data) {
  (void)session;
  (void)flags;

  spdlog::trace("{} onDataChunkRecvCallback()", pthread_self());

  auto holder = *(static_cast<AbstractSession::SessionHolder *>(user_data));
  auto s = holder();

  if (auto cb = s->getOnStateChangeCallback(); cb)
    cb(static_cast<uint8_t>(ReadyState::Loading));

  if (auto strm = s->findStream(stream_id); strm) {
    auto res = strm->response();
    spdlog::trace("{} Call res->onData(data,{}) ", pthread_self(), len);
    res->onData(data, len);
  } else {
    spdlog::trace("{} Stream {} not found ", pthread_self(), stream_id);
  }

  return 0;
}

int onStreamCloseCallback(nghttp2_session *session, int32_t stream_id,
                          uint32_t error_code, void *user_data) {
  (void)session;
  (void)error_code;

  spdlog::trace("{} onStreamCloseCallback()", pthread_self());

  auto holder = *(static_cast<AbstractSession::SessionHolder *>(user_data));
  auto s = holder();

  if (auto strm = s->findStream(stream_id); strm) {
    spdlog::debug("{} Stream{} State::Closed", pthread_self(), strm->id());
    strm->state(Stream::State::Closed);

    if (auto cb = s->getOnStateChangeCallback(); cb)
      cb(static_cast<uint8_t>(ReadyState::Done));

    if (auto cb = s->getOnReadyCallback(); cb)
      cb(std::make_pair(strm->request(), strm->response()));
  }

  spdlog::debug("{} Remove stream {} ", pthread_self(), stream_id);
  s->removeStream(stream_id);

  return 0;
}

ssize_t onSendCallback(nghttp2_session *session, const uint8_t *data,
                       size_t length, int flags, void *user_data) {
  (void)session;
  (void)flags;
  (void)length;
  (void)data;
  (void)user_data;

  spdlog::trace("{} onSendCallback()", pthread_self());

  return 0;
}

int beforeFrameSendCallback(nghttp2_session *session,
                            const nghttp2_frame *frame, void *user_data) {
  (void)session;

  auto processOutgoingFrame = [](const nghttp2_frame *frame, StreamPtr strm) {
    switch (frame->hd.type) {
    case NGHTTP2_DATA: {
      //        (0) The DATA frame.
      spdlog::trace("{} Stream{} DATA frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_HEADERS: {
      //        (0x01) The HEADERS frame.
      spdlog::trace("{} Stream{} HEADERS frame", pthread_self(),
                    frame->hd.stream_id);

      for (size_t n = 0; n < frame->headers.nvlen; n++) {
        auto nameStr = frame->headers.nva[n].name;
        auto nameSz = frame->headers.nva[n].namelen;

        auto valueStr = frame->headers.nva[n].value;
        auto valueSz = frame->headers.nva[n].valuelen;

        strm->request()->header({(char *)nameStr, nameSz},
                                HeaderValue{(char *)valueStr, valueSz});
      }

      break;
    }
    case NGHTTP2_PRIORITY: {
      //        (0x02) The PRIORITY frame.
      spdlog::trace("{} Stream{} PRIORITY frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_RST_STREAM: {
      //        (0x03) The RST_STREAM frame.
      spdlog::trace("{} Stream{} RST_STREAM frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_SETTINGS: {
      //        (0x04) The SETTINGS frame.
      spdlog::trace("{} Stream{} SETTINGS frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_PUSH_PROMISE: {
      //        (0x05) The PUSH_PROMISE frame.
      spdlog::trace("{} Stream{} PUSH_PROMISE frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_PING: {
      //        (0x06) The PING frame.
      spdlog::trace("{} Stream{} PING frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_GOAWAY: {
      //        (0x07) The GOAWAY frame.
      spdlog::trace("{} Stream{} GOAWAY frame", pthread_self(),
                    frame->hd.stream_id);
      strm->state(Stream::State::Closed);
      break;
    }
    case NGHTTP2_WINDOW_UPDATE: {
      //        (0x08) The WINDOW_UPDATE frame.
      spdlog::trace("{} Stream{} WINDOW_UPDATE frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_CONTINUATION: {
      //        (0x09) The CONTINUATION frame. This frame type won't be passed
      //        to any callbacks because the library processes this frame type
      //        and its preceding HEADERS/PUSH_PROMISE as a single frame.
      spdlog::trace("{} Stream{} CONTINUATION frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_ALTSVC: {
      //        (0x0a) The ALTSVC frame, which is defined in RFC 7383.
      spdlog::trace("{} Stream{} ALTSVC frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    case NGHTTP2_ORIGIN: {
      //        (0x0c) The ORIGIN frame, which is defined by RFC 8336
      spdlog::trace("{} Stream{} ORIGIN frame", pthread_self(),
                    frame->hd.stream_id);
      break;
    }
    }
  };

  spdlog::trace("{} beforeFrameSendCallback", pthread_self());

  auto holder = *(static_cast<AbstractSession::SessionHolder *>(user_data));
  auto s = holder();

  if (auto strm = s->findStream(frame->hd.stream_id); strm) {
    spdlog::trace("{} Stream {} found ", pthread_self(), frame->hd.stream_id);
    processOutgoingFrame(frame, strm);
  } else {
    spdlog::trace("{} Stream not found. Create stream {}", pthread_self(),
                  frame->hd.stream_id);
    // Add zero stream when server send setttings[0] and window update
    strm = s->createPushStream(frame->hd.stream_id);
    strm->state(Stream::State::Open);
    processOutgoingFrame(frame, strm);
    s->addStream(std::move(strm));
  }

  return 0;
}

} // namespace network::callbacks
