#pragma once

/**
 * @file
 * @brief TLS callbacks for one server connection.
 * @ingroup tls
 */

#include "ocsp/IOcspProvider.hpp"
#include "tls/IConnectionDetailsProvider.hpp"

#include <botan/asio_stream.h>
#include <botan/tls_callbacks.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Per-connection Botan TLS callbacks: ALPN choice, OCSP stapling and KEX capture.
 *
 * TlsSessionHandler creates one instance per accepted connection and passes it
 * to the Botan::TLS::Stream. After the handshake it is also handed to the
 * protocol handler as the IConnectionDetailsProvider, so
 * GET /api/connection_details reports the group that this client negotiated.
 *
 * The group is recorded in two places. tls_kem_encapsulate() sees it during
 * a KEM handshake. tls_session_established() sees it for every handshake,
 * through Session_Summary::kex_parameters(), and that value wins.
 */
class ServerStreamCallbacks final : public Botan::TLS::StreamCallbacks, public IConnectionDetailsProvider {
   public:
      /**
       * @param alpn_preference Protocols in server preference order.
       * @param ocsp_provider   May be null; then no OCSP response is stapled.
       * @param websocket_port  Port reported as websocket_port in the connection details JSON.
       */
      ServerStreamCallbacks(std::vector<std::string> alpn_preference,
                            std::shared_ptr<IOcspProvider> ocsp_provider,
                            std::uint16_t websocket_port);

      /**
       * @brief Records the group, then delegates to Botan's default encapsulation.
       *
       * Called by Botan when the client's key share uses a KEM group (ML-KEM or a hybrid).
       *
       * @param group              Group of the client's key share.
       * @param encoded_public_key Client's public key share.
       * @param rng                Random number generator for the encapsulation.
       * @param policy             Active TLS policy.
       * @return Botan's encapsulated shared secret and ciphertext.
       */
      Botan::KEM_Encapsulation tls_kem_encapsulate(Botan::TLS::Group_Params group,
                                                   const std::vector<uint8_t>& encoded_public_key,
                                                   Botan::RandomNumberGenerator& rng,
                                                   const Botan::TLS::Policy& policy) override;

      /**
       * @brief Records the negotiated group from the session summary.
       *
       * Uses kex_parameters() (the group name, for example "x25519/ML-KEM-768")
       * and falls back to kex_algo() when Botan provides no group name.
       *
       * @param session Summary of the completed handshake.
       */
      void tls_session_established(const Botan::TLS::Session_Summary& session) override;

      /**
       * @brief Chooses the ALPN protocol with select_alpn_protocol().
       * @param client_protos Protocols offered by the client.
       * @return The chosen protocol, or empty to negotiate none.
       */
      std::string tls_server_choose_app_protocol(const std::vector<std::string>& client_protos) override;

      /**
       * @brief Staples the OCSP response for the server certificate.
       *
       * Asks the OCSP provider using chain[1] as issuer and chain[0] as subject.
       *
       * @param chain Certificate chain sent to the client, server certificate first.
       * @param csr   The client's status request. Ignored.
       * @return The DER-encoded response, or empty when the chain has no issuer, there is no
       *         provider, or no valid response is available.
       */
      std::vector<uint8_t> tls_provide_cert_status(const std::vector<Botan::X509_Certificate>& chain,
                                                   const Botan::TLS::Certificate_Status_Request& csr) override;

      /**
       * @copydoc IConnectionDetailsProvider::connection_details_json
       *
       * Holds the negotiated group, the post-quantum flag, the wire code and
       * the WebSocket port. Before the handshake has completed, kex_algo is
       * "unknown" and wire_code is 0x0000.
       */
      std::string connection_details_json() const override;

   private:
      void remember_group(Botan::TLS::Group_Params group);

      std::vector<std::string> alpn_preference_;
      std::shared_ptr<IOcspProvider> ocsp_provider_;
      std::uint16_t websocket_port_;
      Botan::TLS::Group_Params group_ = Botan::TLS::Group_Params::NONE;
      std::string kex_name_;
};
