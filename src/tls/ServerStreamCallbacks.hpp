#pragma once

#include "ocsp/IOcspProvider.hpp"
#include "tls/IConnectionDetailsProvider.hpp"

#include <botan/asio_stream.h>
#include <botan/tls_callbacks.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/**
 * TLS callbacks of one server connection: picks the ALPN protocol, staples
 * the OCSP response and remembers which key exchange group was negotiated.
 */
class ServerStreamCallbacks final : public Botan::TLS::StreamCallbacks, public IConnectionDetailsProvider {
   public:
      /**
       * @param alpn_preference  Protocols in server preference order.
       * @param ocsp_provider    May be null; then no OCSP response is stapled.
       */
      ServerStreamCallbacks(std::vector<std::string> alpn_preference,
                            std::shared_ptr<IOcspProvider> ocsp_provider,
                            std::uint16_t websocket_port);

      Botan::KEM_Encapsulation tls_kem_encapsulate(Botan::TLS::Group_Params group,
                                                   const std::vector<uint8_t>& encoded_public_key,
                                                   Botan::RandomNumberGenerator& rng,
                                                   const Botan::TLS::Policy& policy) override;

      void tls_session_established(const Botan::TLS::Session_Summary& session) override;

      std::string tls_server_choose_app_protocol(const std::vector<std::string>& client_protos) override;

      std::vector<uint8_t> tls_provide_cert_status(const std::vector<Botan::X509_Certificate>& chain,
                                                   const Botan::TLS::Certificate_Status_Request& csr) override;

      std::string connection_details_json() const override;

   private:
      void remember_group(Botan::TLS::Group_Params group);

      std::vector<std::string> alpn_preference_;
      std::shared_ptr<IOcspProvider> ocsp_provider_;
      std::uint16_t websocket_port_;
      Botan::TLS::Group_Params group_ = Botan::TLS::Group_Params::NONE;
      std::string kex_name_;
};
