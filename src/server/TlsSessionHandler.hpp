#pragma once

/**
 * @file
 * @brief TLS handshake and ALPN dispatch for one connection.
 * @ingroup server
 */

#include "ocsp/IOcspProvider.hpp"
#include "server/ISessionHandler.hpp"

#include <botan/tls_callbacks.h>
#include <botan/tls_policy.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Botan::TLS {
class Context;
}

/// @brief Configuration of a TlsSessionHandler; one per listening port.
struct TlsSessionConfig {
      /// Certificate, key, policy and session manager shared by every connection.
      std::shared_ptr<Botan::TLS::Context> tls_context;

      /// Source of the stapled OCSP response. May be null.
      std::shared_ptr<IOcspProvider> ocsp_provider;

      /// ALPN protocols in server preference order.
      std::vector<std::string> alpn_preference;

      /// Handler per negotiated ALPN protocol.
      std::map<std::string, std::shared_ptr<IProtocolHandler>> protocols;

      /// Used when the negotiated protocol (possibly none) has no entry in @p protocols. May be null.
      std::shared_ptr<IProtocolHandler> fallback;

      /// Port reported to the page as websocket_port in /api/connection_details.
      std::uint16_t websocket_port = 0;
};

/**
 * @brief TLS handshake, then hands the stream to the handler chosen by ALPN and shuts the connection down.
 *
 * For every connection it creates a ServerStreamCallbacks (ALPN, OCSP
 * stapling, group capture) and a Botan::TLS::Stream. After the handshake the
 * negotiated protocol selects the IProtocolHandler. When neither a matching
 * handler nor a fallback exists, the connection is closed without serving
 * anything. Benign disconnects (see is_benign_disconnect()) end the
 * connection quietly.
 *
 * ServerApp builds two instances. The HTTPS port maps "h2" to
 * Http2ProtocolHandler and "http/1.1" to WebSocketProtocolHandler. The
 * --ws-port listener always uses WebSocketProtocolHandler.
 */
class TlsSessionHandler final : public ISessionHandler {
   public:
      /// @param config Port configuration. Copied for the lifetime of the handler.
      explicit TlsSessionHandler(TlsSessionConfig config);

      /**
       * @copydoc ISessionHandler::handle
       *
       * Performs the handshake, dispatches by ALPN and shuts TLS down. The TLS close_notify and the TCP shutdown are attempted even after an
       * error, and their own errors are ignored.
       */
      boost::asio::awaitable<void> handle(AwaitableTcp socket) override;

   private:
      std::shared_ptr<IProtocolHandler> protocol_for(const std::string& alpn) const;

      TlsSessionConfig config_;
};
