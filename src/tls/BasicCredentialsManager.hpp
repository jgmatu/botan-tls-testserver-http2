#pragma once

/**
 * @file
 * @brief Server certificate and key for the TLS handshake.
 * @ingroup tls
 */

#include <botan/credentials_manager.h>
#include <botan/x509cert.h>

#include <memory>
#include <string>
#include <vector>

/**
 * @brief Serves one certificate chain and its private key loaded from PEM files.
 *
 * The chain is loaded once at startup and shared by every connection through
 * the Botan::TLS::Context. The first certificate in the file is the server
 * certificate. The issuer, if present, comes next; OCSP stapling needs it.
 * Client certificates are not requested.
 */
class BasicCredentialsManager final : public Botan::Credentials_Manager {
   public:
      /**
       * @param server_crt PEM file with the server certificate followed by its chain.
       * @param server_key PKCS#8 file with the server's private key.
       * @throws Botan::Exception If a file cannot be read or the key cannot be decoded.
       *         Entries of @p server_crt that are not certificates are skipped.
       */
      BasicCredentialsManager(const std::string& server_crt, const std::string& server_key);

      /**
       * @brief Returns the chain when its key algorithm is acceptable and it matches the SNI name.
       *
       * The chain is used when the key's algorithm is in @p algos and either
       * @p hostname is empty or the server certificate matches it.
       *
       * @param algos                  Key algorithms acceptable for the negotiated signature schemes.
       * @param cert_signature_schemes Ignored.
       * @param acceptable_cas         Ignored.
       * @param type                   Ignored ("tls-server").
       * @param hostname               SNI name sent by the client, empty when none.
       * @return The chain, or an empty vector when it does not fit.
       */
      std::vector<Botan::X509_Certificate> find_cert_chain(
         const std::vector<std::string>& algos,
         const std::vector<Botan::AlgorithmIdentifier>& cert_signature_schemes,
         const std::vector<Botan::X509_DN>& acceptable_cas,
         const std::string& type,
         const std::string& hostname) override;

      /**
       * @brief Private key of the given server certificate.
       * @param cert    Certificate returned earlier by find_cert_chain().
       * @param type    Ignored.
       * @param context Ignored.
       * @return The key, or null for an unknown certificate.
       */
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
