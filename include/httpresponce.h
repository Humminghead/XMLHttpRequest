#pragma once

#include <set>
#include <vector>

#include "abstractresponce.h"

namespace network {
class Responce : public AbstractResponse<Responce> {
private:
  using Chunk = std::vector<uint8_t>;
  using Data = std::vector<Chunk>;

  Data data_;

public:
  void onData(const uint8_t *data, size_t len) override;
  const Data &data() const;

  Responce() = default;

  Responce(const Responce &) = delete;
  Responce &operator=(const Responce &) = delete;
};

} // namespace::network
