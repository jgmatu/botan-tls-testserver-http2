#include "tls/TlsPolicyLoader.hpp"

#include <fstream>
#include <stdexcept>

std::shared_ptr<Botan::TLS::Policy> load_tls_policy(const std::string& policy_type) {
   if(policy_type == "default" || policy_type.empty()) {
      return std::make_shared<Botan::TLS::Policy>();
   }

   // if something we don't recognize, assume it's a file
   std::ifstream policy_stream(policy_type);
   if(!policy_stream.good()) {
      throw std::runtime_error("Unknown TLS policy: not a file or known short name");
   }

   return std::make_shared<Botan::TLS::Text_Policy>(policy_stream);
}
