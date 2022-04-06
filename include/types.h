#pragma once

#include <array>
#include <functional>
#include <memory>
#include <utility>

// clang-format off
namespace network {

class Request;
class Response;

// Session callbacks types
using onReadyCallback = std::function<void(std::pair<std::shared_ptr<Request>, std::shared_ptr<Response>> &&)>;
using onStopCallback = std::function<void()>;
using onConnectCallback = std::function<void()>;
using onStateChangeCallback = std::function<void(const uint8_t)>;
using onTimeoutCallback = std::function<void()>;

// Basic array type
template <size_t N> using Array = std::array<uint8_t, N>;

}
