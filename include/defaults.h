#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <string_view>

// clang-format off
namespace network::defaults {
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
[[maybe_unused]] static constexpr std::string_view default_cipher_list{"\
    TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384:ECDHE-RSA-AES128-GCM-SHA256\
    ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:\
    ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA\
    ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA:AES256-SHA:DES-CBC3-SHA"};
///\brief When session streams empty ping http session every this time (in seconds)
[[maybe_unused]] static constexpr auto ping_time = 30;
} // namespace::network::defaults

#endif // DEFAULTS_H
/*
Cipher Suites (18 suites)
    Cipher Suite: TLS_AES_128_GCM_SHA256 (0x1301)                           TLS_AES_128_GCM_SHA256
    Cipher Suite: TLS_CHACHA20_POLY1305_SHA256 (0x1303)                     TLS_CHACHA20_POLY1305_SHA256
    Cipher Suite: TLS_AES_256_GCM_SHA384 (0x1302)                           TLS_AES_256_GCM_SHA384
    Cipher Suite: TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 (0xc02b)          ECDHE-ECDSA-AES128-GCM-SHA256
    Cipher Suite: TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 (0xc02f)            ECDHE-RSA-AES128-GCM-SHA256
    Cipher Suite: TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 (0xcca9)    ECDHE-ECDSA-CHACHA20-POLY1305
    Cipher Suite: TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 (0xcca8)      ECDHE-RSA-CHACHA20-POLY1305
    Cipher Suite: TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 (0xc02c)          ECDHE-ECDSA-AES256-GCM-SHA384
    Cipher Suite: TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384 (0xc030)            ECDHE-RSA-AES256-GCM-SHA384
    Cipher Suite: TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA (0xc00a)             ECDHE-ECDSA-AES256-SHA
    Cipher Suite: TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA (0xc009)             ECDHE-ECDSA-AES128-SHA
    Cipher Suite: TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA (0xc013)               ECDHE-RSA-AES128-SHA
    Cipher Suite: TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA (0xc014)               ECDHE-RSA-AES256-SHA
    Cipher Suite: TLS_RSA_WITH_AES_128_GCM_SHA256 (0x009c)                  AES128-GCM-SHA256
    Cipher Suite: TLS_RSA_WITH_AES_256_GCM_SHA384 (0x009d)                  AES256-GCM-SHA384
    Cipher Suite: TLS_RSA_WITH_AES_128_CBC_SHA (0x002f)                     AES128-SHA
    Cipher Suite: TLS_RSA_WITH_AES_256_CBC_SHA (0x0035)                     AES256-SHA
    Cipher Suite: TLS_RSA_WITH_3DES_EDE_CBC_SHA (0x000a)                    DES-CBC3-SHA



ECDHE-RSA-AES128-GCM-SHA256
ECDHE-ECDSA-AES128-GCM-SHA256
ECDHE-ECDSA-AES256-GCM-SHA384
*/
