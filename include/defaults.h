#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <string_view>

// clang-format off
namespace network::defaults {
///\brief Default 2Gis server port
[[maybe_unused]] static constexpr std::string_view default_server_port{"443"};
///\brief Default user agent for requests to server
[[maybe_unused]] static constexpr std::string_view user_agent{R"(Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/89.0.4389.114 Safari/537.36)"};
///\brief Default log path
[[maybe_unused]] static constexpr std::string_view network_log_path{R"(/var/log)"};
///\brief Http connect timeout(seconds)
[[maybe_unused]] static constexpr std::size_t http_connect_timeout{0};
///\brief Http read socket timeout(seconds)
[[maybe_unused]] static constexpr std::size_t http_read_timeout{5};
///\brief Payload frame size that the client can receive
[[maybe_unused]] static constexpr std::size_t  http2_max_frame {32768};
///\brief Maximum number of concurrent streams for single client
[[maybe_unused]] static constexpr std::size_t  http2_max_streams{20};
///\brief Default SSL cipher list
[[maybe_unused]] static constexpr std::string_view default_cipher_list{
    R"(TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_CCM_SHA256:TLS_AES_128_CCM_8_SHA256)"};
} // namespace::network::defaults

#endif // DEFAULTS_H
