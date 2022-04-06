#ifndef XMLHTTPREQUEST_H
#define XMLHTTPREQUEST_H

#include "types.h"
#include "xmlhttprequeststate.h"
#include <functional>
#include <memory>
#include <string_view>
#include <utility>

namespace network {

// https://xhr.spec.whatwg.org/
class XMLHttpRequest {
public:
  template <typename T = uint8_t> using ArrayBuffer = std::vector<T>;

  XMLHttpRequest();
  XMLHttpRequest(std::string &&method, std::string &&url, bool async = false);
  XMLHttpRequest(const std::string_view method, const std::string_view url,
                 bool async = false);
  XMLHttpRequest(XMLHttpRequest&& request);
  XMLHttpRequest& operator()(XMLHttpRequest&& request);

  XMLHttpRequest(XMLHttpRequest& request) = delete;
  XMLHttpRequest& operator()(const XMLHttpRequest& request) = delete;

  ~XMLHttpRequest();

  void abort();
  std::string getAllResponseHeaders();
  std::string getResponseHeader(const std::string &header);

  bool open();
  void open(std::string &&method, std::string &&url);
  void open(std::string &&method, std::string &&url, bool async);
  void open(std::string method, std::string uri, bool async,
            const std::string user, const std::string password);
  void overrideMimeType(std::string &&mime);
  void send();
  void send(onReadyCallback &&);
  void send(std::string &&body);

  void setRequestHeader(const std::string &header, const std::string &value);
  void setRequestHeader(std::string &&header, std::string &&value);

  void openRequest(std::string method, std::string url, bool async,
                   std::string user, std::string password);

  bool setOnReadyCallback(onReadyCallback &&cb) noexcept;
  bool setOnStateChangedCallback(onStateChangeCallback &&cb) noexcept;
  void setOnTimeoutCallback(onTimeoutCallback &&) noexcept;

  void timeout(const size_t milliseconds) noexcept;
  size_t timeout() const;

  std::shared_ptr<Response> response() const;

private:
  void initCallbacks() noexcept;
  bool prepare(std::string &&method, std::string &&url, bool async);
  bool setup();
  void setupTimer() noexcept;  

  void sendGet();
  void sendPost(std::string &&body);

  void connect();
  void fetch();
  void fetchWithBody(const std::string &body_);

  struct Impl;
  std::unique_ptr<Impl, void (*)(Impl *)> d;
};

} // namespace network
#endif // XMLHTTPREQUEST_H
