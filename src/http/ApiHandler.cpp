#include "http/ApiHandler.hpp"

#include "common/Strings.hpp"
#include "http/HttpReply.hpp"
#include "http/RequestTarget.hpp"

ApiHandler::ApiHandler(std::shared_ptr<const IConnectionDetailsProvider> connection_details, std::string server) :
      connection_details_(std::move(connection_details)), server_(std::move(server)) {}

void ApiHandler::handle(const H2Request& request, IH2Responder& respond) {
   const std::string_view path = request_path(request.path);

   if(upper_copy(request.method) != "GET") {
      reply(respond, server_, 400, "text/html", "Unknown API method");
      return;
   }

   if(path == "/api/connection_details") {
      reply(respond,
            server_,
            200,
            "application/json",
            connection_details_ ? connection_details_->connection_details_json() : "{}");
      return;
   }

   reply(respond, server_, 404, "text/html", std::string(path));
}
