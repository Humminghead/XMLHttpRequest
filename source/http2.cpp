#include "http2.h"
#include "utilities.h"

namespace network::http2 {

StringRef get_reason_phrase(unsigned int status_code) {
  switch (status_code) {
  case 100:
    return StringRef("Continue");
  case 101:
    return StringRef("Switching Protocols");
  case 103:
    return StringRef("Early Hints");
  case 200:
    return StringRef("OK");
  case 201:
    return StringRef("Created");
  case 202:
    return StringRef("Accepted");
  case 203:
    return StringRef("Non-Authoritative Information");
  case 204:
    return StringRef("No Content");
  case 205:
    return StringRef("Reset Content");
  case 206:
    return StringRef("Partial Content");
  case 300:
    return StringRef("Multiple Choices");
  case 301:
    return StringRef("Moved Permanently");
  case 302:
    return StringRef("Found");
  case 303:
    return StringRef("See Other");
  case 304:
    return StringRef("Not Modified");
  case 305:
    return StringRef("Use Proxy");
  // case 306: return StringRef("(Unused)");
  case 307:
    return StringRef("Temporary Redirect");
  case 308:
    return StringRef("Permanent Redirect");
  case 400:
    return StringRef("Bad Request");
  case 401:
    return StringRef("Unauthorized");
  case 402:
    return StringRef("Payment Required");
  case 403:
    return StringRef("Forbidden");
  case 404:
    return StringRef("Not Found");
  case 405:
    return StringRef("Method Not Allowed");
  case 406:
    return StringRef("Not Acceptable");
  case 407:
    return StringRef("Proxy Authentication Required");
  case 408:
    return StringRef("Request Timeout");
  case 409:
    return StringRef("Conflict");
  case 410:
    return StringRef("Gone");
  case 411:
    return StringRef("Length Required");
  case 412:
    return StringRef("Precondition Failed");
  case 413:
    return StringRef("Payload Too Large");
  case 414:
    return StringRef("URI Too Long");
  case 415:
    return StringRef("Unsupported Media Type");
  case 416:
    return StringRef("Requested Range Not Satisfiable");
  case 417:
    return StringRef("Expectation Failed");
  case 421:
    return StringRef("Misdirected Request");
  case 425:
    // https://tools.ietf.org/html/rfc8470
    return StringRef("Too Early");
  case 426:
    return StringRef("Upgrade Required");
  case 428:
    return StringRef("Precondition Required");
  case 429:
    return StringRef("Too Many Requests");
  case 431:
    return StringRef("Request Header Fields Too Large");
  case 451:
    return StringRef("Unavailable For Legal Reasons");
  case 500:
    return StringRef("Internal Server Error");
  case 501:
    return StringRef("Not Implemented");
  case 502:
    return StringRef("Bad Gateway");
  case 503:
    return StringRef("Service Unavailable");
  case 504:
    return StringRef("Gateway Timeout");
  case 505:
    return StringRef("HTTP Version Not Supported");
  case 511:
    return StringRef("Network Authentication Required");
  default:
    return StringRef{};
  }
}

StringRef stringify_status(unsigned int status_code) {
  switch (status_code) {
  case 100:
    return StringRef("100");
  case 101:
    return StringRef("101");
  case 103:
    return StringRef("103");
  case 200:
    return StringRef("200");
  case 201:
    return StringRef("201");
  case 202:
    return StringRef("202");
  case 203:
    return StringRef("203");
  case 204:
    return StringRef("204");
  case 205:
    return StringRef("205");
  case 206:
    return StringRef("206");
  case 300:
    return StringRef("300");
  case 301:
    return StringRef("301");
  case 302:
    return StringRef("302");
  case 303:
    return StringRef("303");
  case 304:
    return StringRef("304");
  case 305:
    return StringRef("305");
  // case 306: return StringRef("306");
  case 307:
    return StringRef("307");
  case 308:
    return StringRef("308");
  case 400:
    return StringRef("400");
  case 401:
    return StringRef("401");
  case 402:
    return StringRef("402");
  case 403:
    return StringRef("403");
  case 404:
    return StringRef("404");
  case 405:
    return StringRef("405");
  case 406:
    return StringRef("406");
  case 407:
    return StringRef("407");
  case 408:
    return StringRef("408");
  case 409:
    return StringRef("409");
  case 410:
    return StringRef("410");
  case 411:
    return StringRef("411");
  case 412:
    return StringRef("412");
  case 413:
    return StringRef("413");
  case 414:
    return StringRef("414");
  case 415:
    return StringRef("415");
  case 416:
    return StringRef("416");
  case 417:
    return StringRef("417");
  case 421:
    return StringRef("421");
  case 426:
    return StringRef("426");
  case 428:
    return StringRef("428");
  case 429:
    return StringRef("429");
  case 431:
    return StringRef("431");
  case 451:
    return StringRef("451");
  case 500:
    return StringRef("500");
  case 501:
    return StringRef("501");
  case 502:
    return StringRef("502");
  case 503:
    return StringRef("503");
  case 504:
    return StringRef("504");
  case 505:
    return StringRef("505");
  case 511:
    return StringRef("511");
  default:
    return StringRef("Unknown");
  }
}

int parse_http_status_code(const StringRef &src) {
  if (src.size() != 3) {
    return -1;
  }

  int status = 0;
  for (auto c : src) {
    if (!isdigit(c)) {
      return -1;
    }
    status *= 10;
    status += c - '0';
  }

  if (status < 100) {
    return -1;
  }

  return status;
}

int lookup_token(const StringRef &name) {
  return lookup_token((uint8_t *)(name.data()), name.size());
}

// This function was generated by genheaderfunc.py.  Inspired by h2o
// header lookup.  https://github.com/h2o/h2o
int lookup_token(const uint8_t *name, size_t namelen) {
  switch (namelen) {
  case 2:
    switch (name[1]) {
    case 'e':
      if (utils::string::streq_l("t", name, 1)) {
        return HD_TE;
      }
      break;
    }
    break;
  case 3:
    switch (name[2]) {
    case 'a':
      if (utils::string::streq_l("vi", name, 2)) {
        return HD_VIA;
      }
      break;
    }
    break;
  case 4:
    switch (name[3]) {
    case 'e':
      if (utils::string::streq_l("dat", name, 3)) {
        return HD_DATE;
      }
      break;
    case 'k':
      if (utils::string::streq_l("lin", name, 3)) {
        return HD_LINK;
      }
      break;
    case 't':
      if (utils::string::streq_l("hos", name, 3)) {
        return HD_HOST;
      }
      break;
    }
    break;
  case 5:
    switch (name[4]) {
    case 'h':
      if (utils::string::streq_l(":pat", name, 4)) {
        return HD__PATH;
      }
      break;
    case 't':
      if (utils::string::streq_l(":hos", name, 4)) {
        return HD__HOST;
      }
      break;
    }
    break;
  case 6:
    switch (name[5]) {
    case 'e':
      if (utils::string::streq_l("cooki", name, 5)) {
        return HD_COOKIE;
      }
      break;
    case 'r':
      if (utils::string::streq_l("serve", name, 5)) {
        return HD_SERVER;
      }
      break;
    case 't':
      if (utils::string::streq_l("expec", name, 5)) {
        return HD_EXPECT;
      }
      break;
    }
    break;
  case 7:
    switch (name[6]) {
    case 'c':
      if (utils::string::streq_l("alt-sv", name, 6)) {
        return HD_ALT_SVC;
      }
      break;
    case 'd':
      if (utils::string::streq_l(":metho", name, 6)) {
        return HD__METHOD;
      }
      break;
    case 'e':
      if (utils::string::streq_l(":schem", name, 6)) {
        return HD__SCHEME;
      }
      if (utils::string::streq_l("upgrad", name, 6)) {
        return HD_UPGRADE;
      }
      break;
    case 'r':
      if (utils::string::streq_l("traile", name, 6)) {
        return HD_TRAILER;
      }
      break;
    case 's':
      if (utils::string::streq_l(":statu", name, 6)) {
        return HD__STATUS;
      }
      break;
    }
    break;
  case 8:
    switch (name[7]) {
    case 'n':
      if (utils::string::streq_l("locatio", name, 7)) {
        return HD_LOCATION;
      }
      break;
    }
    break;
  case 9:
    switch (name[8]) {
    case 'd':
      if (utils::string::streq_l("forwarde", name, 8)) {
        return HD_FORWARDED;
      }
      break;
    case 'l':
      if (utils::string::streq_l(":protoco", name, 8)) {
        return HD__PROTOCOL;
      }
      break;
    }
    break;
  case 10:
    switch (name[9]) {
    case 'a':
      if (utils::string::streq_l("early-dat", name, 9)) {
        return HD_EARLY_DATA;
      }
      break;
    case 'e':
      if (utils::string::streq_l("keep-aliv", name, 9)) {
        return HD_KEEP_ALIVE;
      }
      break;
    case 'n':
      if (utils::string::streq_l("connectio", name, 9)) {
        return HD_CONNECTION;
      }
      break;
    case 't':
      if (utils::string::streq_l("user-agen", name, 9)) {
        return HD_USER_AGENT;
      }
      break;
    case 'y':
      if (utils::string::streq_l(":authorit", name, 9)) {
        return HD__AUTHORITY;
      }
      break;
    }
    break;
  case 12:
    switch (name[11]) {
    case 'e':
      if (utils::string::streq_l("content-typ", name, 11)) {
        return HD_CONTENT_TYPE;
      }
      break;
    }
    break;
  case 13:
    switch (name[12]) {
    case 'l':
      if (utils::string::streq_l("cache-contro", name, 12)) {
        return HD_CACHE_CONTROL;
      }
      break;
    }
    break;
  case 14:
    switch (name[13]) {
    case 'h':
      if (utils::string::streq_l("content-lengt", name, 13)) {
        return HD_CONTENT_LENGTH;
      }
      break;
    case 's':
      if (utils::string::streq_l("http2-setting", name, 13)) {
        return HD_HTTP2_SETTINGS;
      }
      break;
    }
    break;
  case 15:
    switch (name[14]) {
    case 'e':
      if (utils::string::streq_l("accept-languag", name, 14)) {
        return HD_ACCEPT_LANGUAGE;
      }
      break;
    case 'g':
      if (utils::string::streq_l("accept-encodin", name, 14)) {
        return HD_ACCEPT_ENCODING;
      }
      break;
    case 'r':
      if (utils::string::streq_l("x-forwarded-fo", name, 14)) {
        return HD_X_FORWARDED_FOR;
      }
      break;
    }
    break;
  case 16:
    switch (name[15]) {
    case 'n':
      if (utils::string::streq_l("proxy-connectio", name, 15)) {
        return HD_PROXY_CONNECTION;
      }
      break;
    }
    break;
  case 17:
    switch (name[16]) {
    case 'e':
      if (utils::string::streq_l("if-modified-sinc", name, 16)) {
        return HD_IF_MODIFIED_SINCE;
      }
      break;
    case 'g':
      if (utils::string::streq_l("transfer-encodin", name, 16)) {
        return HD_TRANSFER_ENCODING;
      }
      break;
    case 'o':
      if (utils::string::streq_l("x-forwarded-prot", name, 16)) {
        return HD_X_FORWARDED_PROTO;
      }
      break;
    case 'y':
      if (utils::string::streq_l("sec-websocket-ke", name, 16)) {
        return HD_SEC_WEBSOCKET_KEY;
      }
      break;
    }
    break;
  case 20:
    switch (name[19]) {
    case 't':
      if (utils::string::streq_l("sec-websocket-accep", name, 19)) {
        return HD_SEC_WEBSOCKET_ACCEPT;
      }
      break;
    }
    break;
  }
  return -1;
}

void init_hdidx(HeaderIndex &hdidx) {
  std::fill(std::begin(hdidx), std::end(hdidx), -1);
}

void index_header(HeaderIndex &hdidx, int32_t token, size_t idx) {
  if (token == -1) {
    return;
  }
  assert(token < HD_MAXIDX);
  hdidx[token] = idx;
}

} // namespace network::http2
