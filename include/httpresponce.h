#pragma once

#include "abstractresponce.h"

namespace network {
class Responce : public AbstractResponse<Responce> {
public:
  void onData(const uint8_t *data, size_t len) override;

  Responce() = default;

  Responce(const Responce &) = delete;
  Responce &operator=(const Responce &) = delete;
};

} // namespace network
