#include <spdlog/spdlog.h>

#include <iostream>

#include "httprequest.h"
#include "httpresponce.h"
#include "xmlhttprequest.h"

int main() {
  using namespace network;
  spdlog::set_level(spdlog::level::trace); // Set global log level to debug

  auto method = std::string{"POST"};
  auto url = std::string{"https://httpbin.org/post"};

  XMLHttpRequest req(method, url);

  req.setRequestHeader("accept","*/*");
  req.setRequestHeader("accept-language", "en-US,en;q=0.5");
  req.setRequestHeader("accept-encoding", "gzip, deflate, br");
  req.setRequestHeader("content-length", "0");
  req.setRequestHeader("content-type", "application/x-www-form-urlencoded");
  req.setRequestHeader("host", "httpbin.org");
  req.setRequestHeader("origin","https://httpbin.org/post");
  req.setRequestHeader("refer","https://httpbin.org/post");
  req.setRequestHeader("Sec-Fetch-Dest", "empty");
  req.setRequestHeader("Sec-Fetch-Mode", "cors");
  req.setRequestHeader("Sec-Fetch-Site", "cross-site");
  req.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0");
  req.timeout(3000);

  req.setOnReadyCallback([&](auto&& result) {
      auto [httpRq, httpRp] = result;
      spdlog::info("{} Status code: {}", pthread_self(), httpRp->statusCode());
      spdlog::info("{} Host: {} Method: {} Scheme: {} Response data size: {} ", pthread_self(),
                   httpRq->host(), httpRq->method(), httpRq->scheme(), httpRp->contentLength());

      // Print headers
      spdlog::info("Raw responce headers:\r\n {} ", req.getAllResponseHeaders());      
  });

  req.open();
  req.send();

  return 0;
}
