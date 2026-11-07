#pragma once

/**
 * @file
 * @brief Top-level HTTP application: routes requests to the API or to static files.
 * @ingroup http
 */

#include "http/IRequestHandler.hpp"
#include "http/h2/IH2Application.hpp"

#include <memory>

/**
 * @brief Routes /api* to the API handler and everything else to the static files.
 *
 * This is the IH2Application that H2Session calls for each request. Routing
 * looks only at the path without the query string (see request_path()). One
 * instance is built per connection, because the API handler reports the TLS
 * details of that connection.
 */
class WebApplication final : public IH2Application {
   public:
      /**
       * @param api          Handler for paths that start with "/api".
       * @param static_files Handler for every other path.
       * @throws std::invalid_argument If either handler is null.
       */
      WebApplication(std::shared_ptr<IRequestHandler> api, std::shared_ptr<IRequestHandler> static_files);

      /**
       * @copydoc IH2Application::on_request
       *
       * Forwards the request to the API or the static file handler by path prefix.
       */
      void on_request(const H2Request& request, IH2Responder& respond) override;

   private:
      std::shared_ptr<IRequestHandler> api_;
      std::shared_ptr<IRequestHandler> static_files_;
};
