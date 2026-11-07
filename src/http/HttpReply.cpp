#include "http/HttpReply.hpp"

#include <utility>

void reply(IH2Responder& respond,
           const std::string& server,
           int status,
           std::string content_type,
           std::string body,
           std::vector<H2Header> extra) {
   std::vector<H2Header> headers = {
      {"content-type", std::move(content_type)},
      {"server", server},
   };
   for(auto& header : extra) {
      headers.push_back(std::move(header));
   }
   respond.submit(status, std::move(headers), std::move(body));
}
