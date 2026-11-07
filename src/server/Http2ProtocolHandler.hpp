#pragma once

/**
 * @file
 * @brief IProtocolHandler for HTTP/2 (ALPN h2).
 * @ingroup server
 */

#include "http/IRequestHandler.hpp"
#include "server/ISessionHandler.hpp"

#include <memory>
#include <string>

/**
 * @brief HTTP/2: one WebApplication (API + static files) per connection, driven by H2Session.
 *
 * The static file handler is shared by every connection. The ApiHandler is
 * built per connection from the connection details, so each client sees its
 * own negotiated group. The stack for one connection is
 * H2ConnectionRunner, then H2Session, then WebApplication.
 */
class Http2ProtocolHandler final : public IProtocolHandler {
   public:
      /**
       * @param static_files Handler for everything outside /api.
       * @param server       Value of the "server" response header, for example "Botan 3.14.1".
       */
      Http2ProtocolHandler(std::shared_ptr<IRequestHandler> static_files, std::string server);

      /**
       * @copydoc IProtocolHandler::serve
       *
       * Runs HTTP/2 until the client closes or the connection is idle for 30 seconds.
       */
      boost::asio::awaitable<void> serve(TlsStream& stream,
                                         std::shared_ptr<const IConnectionDetailsProvider> details) override;

   private:
      std::shared_ptr<IRequestHandler> static_files_;
      std::string server_;
};
