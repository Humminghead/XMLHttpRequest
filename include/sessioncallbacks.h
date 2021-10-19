#pragma once

#include <nghttp2/nghttp2.h>

namespace network::callbacks {

[[maybe_unused]] int onBeginHeadersCallback(nghttp2_session *session,
                                            const nghttp2_frame *frame,
                                            void *user_data);

[[maybe_unused]] int onHeaderCallback(nghttp2_session *session,
                                      const nghttp2_frame *frame,
                                      const uint8_t *name, size_t namelen,
                                      const uint8_t *value, size_t valuelen,
                                      uint8_t flags, void *user_data);

[[maybe_unused]] int onFrameRecvCallback(nghttp2_session *session,
                                         const nghttp2_frame *frame,
                                         void *user_data);

[[maybe_unused]] int onDataChunkRecvCallback(nghttp2_session *session,
                                             uint8_t flags, int32_t stream_id,
                                             const uint8_t *data, size_t len,
                                             void *user_data);

[[maybe_unused]] int onStreamCloseCallback(nghttp2_session *session,
                                           int32_t stream_id,
                                           uint32_t error_code,
                                           void *user_data);

[[maybe_unused]] ssize_t onSendCallback(nghttp2_session *session,
                                        const uint8_t *data, size_t length,
                                        int flags, void *user_data);

[[maybe_unused]] int beforeFrameSendCallback(nghttp2_session *session,
                                             const nghttp2_frame *frame,
                                             void *user_data);

} // namespace network::callbacks
