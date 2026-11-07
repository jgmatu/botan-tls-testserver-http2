#include "tls/BasicCredentialsManager.hpp"

#include <botan/data_src.h>
#include <botan/pkcs8.h>

#include <algorithm>
#include <ranges>

BasicCredentialsManager::BasicCredentialsManager(const std::string& server_crt, const std::string& server_key) {
   CertificateInfo cert;

   Botan::DataSource_Stream key_in(server_key);
   cert.key = Botan::PKCS8::load_key(key_in);

   Botan::DataSource_Stream in(server_crt);
   while(!in.end_of_data()) {
      try {
         cert.certs.push_back(Botan::X509_Certificate(in));
      } catch(std::exception&) {}
   }

   creds_.push_back(cert);
}

std::vector<Botan::X509_Certificate> BasicCredentialsManager::find_cert_chain(
   const std::vector<std::string>& algos,
   const std::vector<Botan::AlgorithmIdentifier>& /*cert_signature_schemes*/,
   const std::vector<Botan::X509_DN>& /*acceptable_cas*/,
   const std::string& /*type*/,
   const std::string& hostname) {
   const auto cred = std::ranges::find_if(creds_, [&](const auto& cred) {
      return std::ranges::any_of(algos, [&](const auto& algo) { return algo == cred.key->algo_name(); }) &&
             (hostname.empty() || cred.certs.front().matches_dns_name(hostname));
   });

   return (cred != creds_.end()) ? cred->certs : std::vector<Botan::X509_Certificate>{};
}

std::shared_ptr<Botan::Private_Key> BasicCredentialsManager::private_key_for(const Botan::X509_Certificate& cert,
                                                                             const std::string& /*type*/,
                                                                             const std::string& /*context*/) {
   const auto cred = std::ranges::find_if(creds_, [&](const auto& cred) { return cred.certs.front() == cert; });
   return (cred != creds_.end()) ? cred->key : nullptr;
}
