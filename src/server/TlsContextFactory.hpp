#pragma once

#include "server/ServerOptions.hpp"

#include <botan/tls_callbacks.h>

#include <memory>

namespace Botan::TLS {
class Context;
}

/**
 * TLS server context from the certificate, key and policy of the options.
 * Session resumption is off: a second connection (the WebSocket) would
 * otherwise try to resume the HTTP session and Firefox aborts the hybrid
 * ML-KEM handshake.
 */
std::shared_ptr<Botan::TLS::Context> create_tls_context(const ServerOptions& options);
