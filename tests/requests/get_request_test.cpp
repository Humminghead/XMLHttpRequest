#include "httprequest.h"
#include "httpresponce.h"
#include "xmlhttprequest.h"

#include <iostream>
#include <spdlog/spdlog.h>

int main() {
  using namespace network;

  XMLHttpRequest rq( //
      "GET", "https://mapgl.2gis.com/api/fonts/Noto_Sans_4.pbf", true);

  rq.open();

  // clang-format off
      rq.setRequestHeader("accept", "*/*");
      rq.setRequestHeader("accept-encoding", "gzip, deflate, br");
      rq.setRequestHeader("accept-language", "ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7");
      rq.setRequestHeader("cache-control", "no-cache");
      rq.setRequestHeader("dnt", "1");
      rq.setRequestHeader("host", "mapgl.2gis.com");
      rq.setRequestHeader("origin", "https://2gis.ru");
      rq.setRequestHeader("pragma", "no-cache");
      rq.setRequestHeader("referer", "https://2gis.ru/");
      rq.setRequestHeader("user-agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0");
  // clang-format on

  // Send with callback
  rq.send([](auto &&result) {
    if (auto [rq, rp] = result; 200 == rp->statusCode()) {
      spdlog::info("{} Host: {} Method: {} Scheme: {} Responce data size: {} ",
                   pthread_self(), rq->host(), rq->method(), rq->scheme(),
                   rp->contentLength());
    } else {
      spdlog::info("{} Code: {}", pthread_self(), rp->statusCode());
    }
  });

  // Wait PING
  std::this_thread::sleep_for(std::chrono::seconds(32));

  // Stop
  rq.abort();

  // Print headers
  spdlog::info("Raw responce headers:\r\n {} ", rq.getAllResponseHeaders());

  std::cout << "Hello from test!" << std::endl;
  return 0;
}
