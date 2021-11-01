#include "httpresponce.h"

namespace network {
void Responce::onData(const uint8_t *data, size_t len) {
  auto chunk = chunk_type{};
  chunk.reserve(len);
  std::copy(data, data + len, std::back_inserter(chunk));
  contentLengthInc(len);
  data_.push_back(std::move(chunk));
}

} // namespace network
