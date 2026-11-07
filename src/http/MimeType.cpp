#include "http/MimeType.hpp"

#include "common/Strings.hpp"

std::string mime_type(std::string_view path) {
   const auto ext = [&path] {
      const auto pos = path.rfind('.');
      if(pos == std::string_view::npos) {
         return std::string_view{};
      }
      return path.substr(pos);
   }();

   if(iequals(ext, ".htm") || iequals(ext, ".html")) {
      return "text/html";
   }
   if(iequals(ext, ".css")) {
      return "text/css";
   }
   if(iequals(ext, ".txt")) {
      return "text/plain";
   }
   if(iequals(ext, ".js")) {
      return "application/javascript";
   }
   if(iequals(ext, ".json")) {
      return "application/json";
   }
   if(iequals(ext, ".png") || iequals(ext, ".ico")) {
      return "image/png";
   }
   if(iequals(ext, ".jpe") || iequals(ext, ".jpeg") || iequals(ext, ".jpg")) {
      return "image/jpeg";
   }
   if(iequals(ext, ".gif")) {
      return "image/gif";
   }
   if(iequals(ext, ".svg") || iequals(ext, ".svgz")) {
      return "image/svg+xml";
   }
   return "application/text";
}
