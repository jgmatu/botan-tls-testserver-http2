#pragma once

/**
 * @file
 * @brief Construction of the shared Botan TLS context.
 * @ingroup server
 */

#include "server/ServerOptions.hpp"

#include <botan/tls_callbacks.h>

#include <memory>

namespace Botan::TLS {
class Context;
}

/**
 * @brief TLS server context from the certificate, key and policy of the options.
 *
 * Session resumption is off: a second connection (the WebSocket) would
 * otherwise try to resume the HTTP session and Firefox aborts the hybrid
 * ML-KEM handshake.
 *
 * The context holds a BasicCredentialsManager, an AutoSeeded_RNG, a
 * Session_Manager_Noop and the policy from load_tls_policy(). Both listeners
 * share it.
 *
 * @param options Uses cert, key and policy.
 * @return The context.
 * @throws std::exception If the certificate, key or policy cannot be loaded.
 */
std::shared_ptr<Botan::TLS::Context> create_tls_context(const ServerOptions& options);
