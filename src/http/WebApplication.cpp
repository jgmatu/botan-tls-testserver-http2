#include "http/WebApplication.hpp"

#include "http/RequestTarget.hpp"

#include <stdexcept>

WebApplication::WebApplication(std::shared_ptr<IRequestHandler> api, std::shared_ptr<IRequestHandler> static_files) :
      api_(std::move(api)), static_files_(std::move(static_files)) {
   if(!api_ || !static_files_) {
      throw std::invalid_argument("WebApplication requires both request handlers");
   }
}

void WebApplication::on_request(const H2Request& request, IH2Responder& respond) {
   if(request_path(request.path).starts_with("/api")) {
      api_->handle(request, respond);
   } else {
      static_files_->handle(request, respond);
   }
}
