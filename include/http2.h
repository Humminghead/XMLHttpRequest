#pragma once

#include <array>
#include <cstdio>
#include <cstring>
#include <nghttp2/nghttp2.h>
#include <string>
#include <vector>

struct http_parser_url;

namespace network::http2 {

// Header fields to be indexed, except HD_MAXIDX which is convenient
// member to get maximum value.
enum {
  HD__AUTHORITY,
  HD__HOST,
  HD__METHOD,
  HD__PATH,
  HD__PROTOCOL,
  HD__SCHEME,
  HD__STATUS,
  HD_ACCEPT_ENCODING,
  HD_ACCEPT_LANGUAGE,
  HD_ALT_SVC,
  HD_CACHE_CONTROL,
  HD_CONNECTION,
  HD_CONTENT_LENGTH,
  HD_CONTENT_TYPE,
  HD_COOKIE,
  HD_DATE,
  HD_EARLY_DATA,
  HD_EXPECT,
  HD_FORWARDED,
  HD_HOST,
  HD_HTTP2_SETTINGS,
  HD_IF_MODIFIED_SINCE,
  HD_KEEP_ALIVE,
  HD_LINK,
  HD_LOCATION,
  HD_PROXY_CONNECTION,
  HD_SEC_WEBSOCKET_ACCEPT,
  HD_SEC_WEBSOCKET_KEY,
  HD_SERVER,
  HD_TE,
  HD_TRAILER,
  HD_TRANSFER_ENCODING,
  HD_UPGRADE,
  HD_USER_AGENT,
  HD_VIA,
  HD_X_FORWARDED_FOR,
  HD_X_FORWARDED_PROTO,
  HD_MAXIDX,
};

using HeaderIndex = std::array<int16_t, HD_MAXIDX>;

using StringRef = std::string_view;

StringRef stringify_status(unsigned int status_code);

StringRef get_reason_phrase(unsigned int status_code);

// Returns parsed HTTP status code.  Returns -1 on failure.
int parse_http_status_code(const StringRef &src);

// Looks up header token for header name |name| of length |namelen|.
// Only headers we are interested in are tokenized.  If header name
// cannot be tokenized, returns -1.
int lookup_token(const uint8_t *name, size_t namelen);
int lookup_token(const StringRef &name);

} // namespace network::http2
