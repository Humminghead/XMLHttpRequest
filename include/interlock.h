#pragma once

#include <atomic>

namespace network {

class Interlock {
private:
  std::atomic_bool flag_;

public:
  Interlock() noexcept = default;
  ~Interlock() = default;

  Interlock(const Interlock &) = delete;
  Interlock &operator=(const Interlock &) = delete;

  void lock() {
    flag_.store(true);

    //    if (flag_.load())
    //      throw std::logic_error("Already locked!");
  }

  bool tryLock() noexcept {
    if (!flag_.load()) {
      flag_.store(true);
      return true;
    }

    return false;
  }

  void unlock() { flag_.store(false); }

  bool isLocked() { return (true == flag_.load()); }
};

} // namespace::network
