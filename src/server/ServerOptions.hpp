#pragma once

/**
 * @file
 * @brief Server settings from the command line.
 * @ingroup server
 */

#include <cstdint>
#include <string>

/// @brief Validated settings for ServerApp. Defaults match the command line defaults.
struct ServerOptions {
      std::uint16_t port = 0;                            ///< HTTPS port (--port): HTTP/2 and the same-port WebSocket.
      std::uint16_t ws_port = 50444;                     ///< Extra WebSocket port (--ws-port) for non-browser clients.
      std::string policy = "default";                    ///< TLS policy file or "default" (--policy).
      std::string cert;                                  ///< PEM certificate chain (--cert).
      std::string key;                                   ///< PKCS#8 private key (--key).
      std::uint64_t ocsp_request_timeout_seconds = 10;   ///< OCSP HTTP timeout (--ocsp-request-timeout).
      std::uint64_t ocsp_cache_minutes = 6 * 60;         ///< Maximum OCSP cache time (--ocsp-cache-time).
      std::string document_root = "webroot";             ///< Directory of the static files (--document-root).
};
