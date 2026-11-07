#pragma once

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

struct TlsSessionConfig {
      std::shared_ptr<Botan::TLS::Context> tls_context;
      std::shared_ptr<IOcspProvider> ocsp_provider;

      /// ALPN protocols in server preference order.
      std::vector<std::string> alpn_preference;

      /// Handler per negotiated ALPN protocol.
      std::map<std::string, std::shared_ptr<IProtocolHandler>> protocols;

      /// Used when the negotiated protocol (possibly none) has no entry in @p protocols. May be null.
      std::shared_ptr<IProtocolHandler> fallback;

      std::uint16_t websocket_port = 0;
};

/// TLS handshake, then hands the stream to the handler chosen by ALPN and shuts the connection down.
class TlsSessionHandler final : public ISessionHandler {
   public:
      explicit TlsSessionHandler(TlsSessionConfig config);

      boost::asio::awaitable<void> handle(AwaitableTcp socket) override;

   private:
      std::shared_ptr<IProtocolHandler> protocol_for(const std::string& alpn) const;

      TlsSessionConfig config_;
};
