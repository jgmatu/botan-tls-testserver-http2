#include "http/StaticFileHandler.hpp"

#include "common/Strings.hpp"
#include "http/HttpReply.hpp"
#include "http/MimeType.hpp"
#include "http/RequestTarget.hpp"

StaticFileHandler::StaticFileHandler(std::string document_root,
                                     std::shared_ptr<const IFileSystem> file_system,
                                     std::string server) :
      document_root_(std::move(document_root)), file_system_(std::move(file_system)), server_(std::move(server)) {}

void StaticFileHandler::handle(const H2Request& request, IH2Responder& respond) {
   const std::string method = upper_copy(request.method);
   const std::string_view path = request_path(request.path);

   if(method != "GET" && method != "HEAD") {
      reply(respond, server_, 400, "text/html", "Unknown HTTP-method");
      return;
   }

   if(!is_safe_request_path(path)) {
      reply(respond, server_, 400, "text/html", "Illegal request-target");
      return;
   }

   const std::string file = resolve_file_path(document_root_, path);

   std::error_code ec;
   const FileType type = file_system_->type(file, ec);
   if(ec) {
      reply(respond, server_, 500, "text/html", std::string("An error occured: ") + ec.message());
      return;
   }
   if(type == FileType::NotFound) {
      reply(respond, server_, 404, "text/html", std::string(path));
      return;
   }
   if(type != FileType::Regular) {
      reply(respond, server_, 500, "text/html", "An error occured: Not a file");
      return;
   }

   const auto size = file_system_->size(file, ec);
   if(ec) {
      reply(respond, server_, 500, "text/html", std::string("An error occured: ") + ec.message());
      return;
   }

   const std::string content_type = mime_type(file);
   if(method == "HEAD") {
      reply(respond, server_, 200, content_type, {}, {{"content-length", std::to_string(size)}});
      return;
   }

   std::string body;
   if(!file_system_->read_all(file, body)) {
      reply(respond, server_, 500, "text/html", "An error occured: failed to read file");
      return;
   }
   reply(respond, server_, 200, content_type, std::move(body));
}
