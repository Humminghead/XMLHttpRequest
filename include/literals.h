#pragma once

// User-defined literals for kilobites, megabites, and gigabutes (powers of
// 1024)
namespace network::literals {

constexpr unsigned long long operator"" _Kb(unsigned long long k) {
  return k * 1024;
}

constexpr unsigned long long operator"" _Mb(unsigned long long m) {
  return m * 1024 * 1024;
}

constexpr unsigned long long operator"" _Gb(unsigned long long g) {
  return g * 1024 * 1024 * 1024;
}
} // namespace network::literals
