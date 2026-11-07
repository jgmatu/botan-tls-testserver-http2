#include "tls/ServerStreamCallbacks.hpp"

#include "tls/AlpnSelector.hpp"
#include "tls/ConnectionDetails.hpp"

#include <botan/tls_session.h>

ServerStreamCallbacks::ServerStreamCallbacks(std::vector<std::string> alpn_preference,
                                             std::shared_ptr<IOcspProvider> ocsp_provider,
                                             std::uint16_t websocket_port) :
      alpn_preference_(std::move(alpn_preference)),
      ocsp_provider_(std::move(ocsp_provider)),
      websocket_port_(websocket_port) {}

Botan::KEM_Encapsulation ServerStreamCallbacks::tls_kem_encapsulate(Botan::TLS::Group_Params group,
                                                                    const std::vector<uint8_t>& encoded_public_key,
                                                                    Botan::RandomNumberGenerator& rng,
                                                                    const Botan::TLS::Policy& policy) {
   remember_group(group);
   return Botan::TLS::StreamCallbacks::tls_kem_encapsulate(group, encoded_public_key, rng, policy);
}

void ServerStreamCallbacks::tls_session_established(const Botan::TLS::Session_Summary& session) {
   if(const auto params = session.kex_parameters()) {
      kex_name_ = *params;
      if(const auto group = Botan::TLS::Group_Params::from_string(*params)) {
         remember_group(*group);
      }
   } else if(!session.kex_algo().empty()) {
      kex_name_ = session.kex_algo();
   }
   Botan::TLS::StreamCallbacks::tls_session_established(session);
}

std::string ServerStreamCallbacks::tls_server_choose_app_protocol(const std::vector<std::string>& client_protos) {
   return select_alpn_protocol(client_protos, alpn_preference_);
}

std::vector<uint8_t> ServerStreamCallbacks::tls_provide_cert_status(const std::vector<Botan::X509_Certificate>& chain,
                                                                    const Botan::TLS::Certificate_Status_Request&) {
   if(chain.size() < 2 || !ocsp_provider_) {
      return {};
   }

   return ocsp_provider_->get_ocsp_response(chain[1], chain[0]);
}

std::string ServerStreamCallbacks::connection_details_json() const {
   ConnectionDetails details;
   details.kex_algo = !kex_name_.empty() ? kex_name_ : group_.to_string().value_or("unknown");
   details.is_quantum_safe = group_.is_post_quantum();
   details.wire_code = group_.wire_code();
   details.websocket_port = websocket_port_;
   return to_json(details);
}

void ServerStreamCallbacks::remember_group(Botan::TLS::Group_Params group) {
   group_ = group;
   if(const auto name = group.to_string()) {
      kex_name_ = *name;
   }
}
