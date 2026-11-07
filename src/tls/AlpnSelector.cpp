#include "tls/AlpnSelector.hpp"

#include <algorithm>

std::string select_alpn_protocol(const std::vector<std::string>& client_protocols,
                                 const std::vector<std::string>& server_preference) {
   for(const std::string& proto : server_preference) {
      if(std::ranges::find(client_protocols, proto) != client_protocols.end()) {
         return proto;
      }
   }
   return {};
}
