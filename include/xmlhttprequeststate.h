#pragma once

#include <string_view>

namespace network {

enum class ReadyState : uint8_t {
  Unsent = 0,
  Opened,
  HeadersReceived,
  Loading,
  Done
};

constexpr std::string_view readyStateToString(const uint8_t state) {
  switch (state) {
  case static_cast<uint8_t>(ReadyState::Done):
    return "Done";
  case static_cast<uint8_t>(ReadyState::HeadersReceived):
    return "HeadersReceived";
  case static_cast<uint8_t>(ReadyState::Loading):
    return "Loading";
  case static_cast<uint8_t>(ReadyState::Opened):
    return "Opened";
  case static_cast<uint8_t>(ReadyState::Unsent):
    return "Unsent";
  }
  return "Error while state conversion! Unknown state!";
}

} // namespace::network
