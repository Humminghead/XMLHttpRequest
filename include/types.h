#pragma once

#include <functional>
#include <utility>
#include <memory>
//#include <vector>

namespace network {

class Request;
class Responce;

// Session callbacks types
using onReadyCallback = std::function<void(
    std::pair<std::shared_ptr<Request>, std::shared_ptr<Responce>> &&)>;
using onStopCallback = std::function<void()>;
using onConnectCallback = std::function<void()>;
using onStateChangeCallback = std::function<void(const uint8_t)>;
using onTimeoutCallback = std::function<void()>;

//template <typename T = uint8_t> using BufferType = std::vector<T>;
}
