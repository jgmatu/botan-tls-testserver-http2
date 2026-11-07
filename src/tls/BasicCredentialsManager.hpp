#pragma once

#include <botan/credentials_manager.h>
#include <botan/x509cert.h>

#include <memory>
#include <string>
#include <vector>

/// Serves one certificate chain and its private key loaded from PEM files.
class BasicCredentialsManager final : public Botan::Credentials_Manager {
   public:
      BasicCredentialsManager(const std::string& server_crt, const std::string& server_key);

      std::vector<Botan::X509_Certificate> find_cert_chain(
         const std::vector<std::string>& algos,
         const std::vector<Botan::AlgorithmIdentifier>& cert_signature_schemes,
         const std::vector<Botan::X509_DN>& acceptable_cas,
         const std::string& type,
         const std::string& hostname) override;

      std::shared_ptr<Botan::Private_Key> private_key_for(const Botan::X509_Certificate& cert,
                                                          const std::string& type,
                                                          const std::string& context) override;

   private:
      struct CertificateInfo {
            std::vector<Botan::X509_Certificate> certs;
            std::shared_ptr<Botan::Private_Key> key;
      };

      std::vector<CertificateInfo> creds_;
};
