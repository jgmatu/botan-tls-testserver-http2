#pragma once

/**
 * @file
 * @brief OCSP client over plain HTTP/1.1 with Boost.Beast.
 * @ingroup ocsp
 */

#include "common/ILogger.hpp"
#include "ocsp/IOcspFetcher.hpp"

#include <chrono>
#include <memory>

/**
 * @brief Plain HTTP OCSP client built on Boost.Beast.
 *
 * Takes the responder URL from the subject certificate's AIA extension and
 * POSTs a DER OCSP request with content type application/ocsp-request. The
 * request is synchronous: it runs its own io_context and blocks the calling
 * thread for up to the timeout. It does not use TLS, so an https:// responder
 * is contacted in plain text on port 443.
 *
 * This is the only part of the project that still uses Beast; the server side
 * uses nghttp2.
 */
class BeastOcspFetcher final : public IOcspFetcher {
   public:
      /**
       * @param request_timeout Limit for connecting, writing and reading, from --ocsp-request-timeout.
       * @param logger          Receives the responder URL before each request.
       */
      BeastOcspFetcher(std::chrono::seconds request_timeout, std::shared_ptr<ILogger> logger);

      /**
       * @brief Sends the OCSP request and parses the answer.
       * @param issuer  Certificate that issued @p subject.
       * @param subject Certificate whose status is requested; its AIA gives the responder URL.
       * @return The response, or null when the certificate has no responder, its URL cannot be
       *         parsed, or the HTTP status is not 200.
       * @throws boost::system::system_error On DNS, connection, timeout or I/O failure.
       * @throws Botan::Exception            When the body is not a valid OCSP response.
       */
      std::unique_ptr<IOcspResponse> fetch(const Botan::X509_Certificate& issuer,
                                           const Botan::X509_Certificate& subject) override;

   private:
      std::chrono::seconds request_timeout_;
      std::shared_ptr<ILogger> logger_;
};
