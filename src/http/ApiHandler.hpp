#pragma once

/**
 * @file
 * @brief REST API of the test server.
 * @ingroup http
 */

#include "http/IRequestHandler.hpp"
#include "tls/IConnectionDetailsProvider.hpp"

#include <memory>
#include <string>

/**
 * @brief REST API: currently only GET /api/connection_details.
 *
 * GET /api/connection_details returns the JSON from
 * IConnectionDetailsProvider::connection_details_json() for the connection
 * the request came on: the negotiated key exchange group, whether it is
 * post-quantum, its wire code and the WebSocket port.
 *
 * Any other method is answered with 400 and any other /api path with 404.
 */
class ApiHandler final : public IRequestHandler {
   public:
      /**
       * @param connection_details TLS details of this connection. When null the endpoint returns "{}".
       * @param server             Value of the "server" response header.
       */
      ApiHandler(std::shared_ptr<const IConnectionDetailsProvider> connection_details, std::string server);

      /// @copydoc IRequestHandler::handle
      void handle(const H2Request& request, IH2Responder& respond) override;

   private:
      std::shared_ptr<const IConnectionDetailsProvider> connection_details_;
      std::string server_;
};
