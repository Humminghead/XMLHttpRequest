#include <spdlog/spdlog.h>

#include <iostream>

#include "httprequest.h"
#include "httpresponce.h"
#include "xmlhttprequest.h"

int main() {
  using namespace network;
  spdlog::set_level(spdlog::level::trace); // Set global log level to debug

  XMLHttpRequest
      req( //
           //    "GET", "https://mapgl.2gis.com/api/fonts/Noto_Sans_4.pbf",
           //    false);
          "GET", "https://ikus.pesc.ru/styles.097d94b829258e90f364.css", false);

  // clang-format off
//          req.setRequestHeader("accept", "*/*");
//          req.setRequestHeader("accept-encoding", "gzip, deflate, br");
//          req.setRequestHeader("accept-language", "ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7");
//          req.setRequestHeader("cache-control", "no-cache");
//          req.setRequestHeader("dnt", "1");
//          req.setRequestHeader("host", "mapgl.2gis.com");
//          req.setRequestHeader("origin", "https://2gis.ru");
//          req.setRequestHeader("pragma", "no-cache");
//          req.setRequestHeader("referer", "https://2gis.ru/");
//          req.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0");
  // clang-format on

  // clang-format off
          req.setRequestHeader("accept", "text/css,*/*;q=0.1");
          req.setRequestHeader("accept-encoding", "gzip, deflate, br");
          req.setRequestHeader("accept-language", "en-US,en;q=0.5");
          req.setRequestHeader("cache-control", "no-cache");
          req.setRequestHeader("dnt", "1");
          req.setRequestHeader("host", "ikus.pesc.ru");
  //        req.setRequestHeader("origin", "https://2gis.ru");
          req.setRequestHeader("pragma", "no-cache");
          req.setRequestHeader("referer", "https://ikus.pesc.ru/login");
          req.setRequestHeader("Sec-Fetch-Dest", "style");
          req.setRequestHeader("Sec-Fetch-Mode", "no-cors");
          req.setRequestHeader("Sec-Fetch-Site", "same-origin");
          req.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0");
  // clang-format on
  req.timeout(1000);
  req.open();
  //  req.overrideMimeType("text/plain; charset=x-user-defined");

  // Send with callback
  req.send([&](auto &&result) {
    if (auto [httpRq, httpRp] = result; 200 == httpRp->statusCode()) {
      spdlog::info("{} Status code: {}", pthread_self(), httpRp->statusCode());
      spdlog::info("{} Host: {} Method: {} Scheme: {} Responce data size: {} ",
                   pthread_self(), httpRq->host(), httpRq->method(),
                   httpRq->scheme(), httpRp->contentLength());
    } else {
      spdlog::info("{} Status code: {}", pthread_self(), httpRp->statusCode());
    }

    // Print headers
    spdlog::info("Raw responce headers:\r\n {} ", req.getAllResponseHeaders());

    // Stop
    req.abort();
  });

  return 0;
}
