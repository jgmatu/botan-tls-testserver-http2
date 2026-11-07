#pragma once

#include "http/IRequestHandler.hpp"
#include "tls/IConnectionDetailsProvider.hpp"

#include <memory>
#include <string>

/// REST API: currently only GET /api/connection_details.
class ApiHandler final : public IRequestHandler {
   public:
      ApiHandler(std::shared_ptr<const IConnectionDetailsProvider> connection_details, std::string server);

      void handle(const H2Request& request, IH2Responder& respond) override;

   private:
      std::shared_ptr<const IConnectionDetailsProvider> connection_details_;
      std::string server_;
};
