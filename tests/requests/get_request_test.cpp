#include <spdlog/spdlog.h>

#include <iostream>

#include "httprequest.h"
#include "httpresponse.h"
#include "xmlhttprequest.h"

int main() {
  using namespace network;
  spdlog::set_level(spdlog::level::trace); // Set global log level to debug

  auto method = std::string{"GET"};
  auto url = std::string{"https://github.githubassets.com/images/mona-loading-dark.gif"};

  XMLHttpRequest req(method, url);

  req.setRequestHeader("accept","image/avif,image/webp,*/*");
  req.setRequestHeader("accept-encoding", "gzip, deflate, br");
  req.setRequestHeader("accept-language", "en-US,en;q=0.5");
  req.setRequestHeader("cache-control", "no-cache");
  req.setRequestHeader("pragma", "no-cache");
  req.setRequestHeader("host", "github.githubassets.com");
  req.setRequestHeader("Sec-Fetch-Dest", "image");
  req.setRequestHeader("Sec-Fetch-Mode", "no-cors");
  req.setRequestHeader("Sec-Fetch-Site", "cross-site");
  req.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0");  
  req.timeout(3000);
  req.open();

  // Send with callback
  req.send([&](auto &&result) {
    if (auto [httpRq, httpRp] = result; 200 == httpRp->statusCode()) {
      spdlog::info("{} Status code: {}", pthread_self(), httpRp->statusCode());
      spdlog::info("{} Host: {} Method: {} Scheme: {} Response data size: {} ",
                   pthread_self(), httpRq->host(), httpRq->method(),
                   httpRq->scheme(), httpRp->contentLength());
    } else {
      spdlog::info("{} Status code: {}", pthread_self(), httpRp->statusCode());
    }

    // Print headers
    spdlog::info("Raw response headers:\r\n {} ", req.getAllResponseHeaders());

    // Stop
    req.abort();
  });

  return 0;
}
