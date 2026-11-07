#include "ocsp/Url.hpp"

std::optional<Url> parse_url(const std::string& urlstr) {
   const auto protocol_host_sep = urlstr.find("://");
   if(protocol_host_sep == std::string::npos) {
      return {};
   }

   const auto host_target_sep = urlstr.find('/', protocol_host_sep + 3);

   Url url;

   if(host_target_sep == std::string::npos) {
      url.host = urlstr.substr(protocol_host_sep + 3, std::string::npos);
      url.target = "/";
   } else {
      url.host = urlstr.substr(protocol_host_sep + 3, host_target_sep - protocol_host_sep - 3);
      url.target = urlstr.substr(host_target_sep, std::string::npos);
   }

   const auto port_sep = url.host.find(':');
   if(port_sep == std::string::npos) {
      url.protocol = "http";
      // hostname not modified
   } else {
      url.port = url.host.substr(port_sep + 1, std::string::npos);
      url.host = url.host.substr(0, port_sep);
   }

   return url;
}
