#pragma once

#include "http/IRequestHandler.hpp"
#include "http/h2/IH2Application.hpp"

#include <memory>

/// Routes /api/* to the API handler and everything else to the static files.
class WebApplication final : public IH2Application {
   public:
      WebApplication(std::shared_ptr<IRequestHandler> api, std::shared_ptr<IRequestHandler> static_files);

      void on_request(const H2Request& request, IH2Responder& respond) override;

   private:
      std::shared_ptr<IRequestHandler> api_;
      std::shared_ptr<IRequestHandler> static_files_;
};
