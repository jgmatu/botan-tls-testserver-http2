#pragma once

#include "ocsp/IOcspFetcher.hpp"
#include "ocsp/IOcspProvider.hpp"
#include "ocsp/IOcspResponse.hpp"

#include <gmock/gmock.h>

class MockOcspResponse : public IOcspResponse {
   public:
      MOCK_METHOD(bool, successful, (), (const, override));
      MOCK_METHOD(Botan::Certificate_Status_Code,
                  status_for,
                  (const Botan::X509_Certificate& issuer,
                   const Botan::X509_Certificate& subject,
                   std::chrono::system_clock::time_point ref_time),
                  (const, override));
      MOCK_METHOD(std::vector<std::uint8_t>, raw_bits, (), (const, override));
};

class MockOcspFetcher : public IOcspFetcher {
   public:
      MOCK_METHOD(std::unique_ptr<IOcspResponse>,
                  fetch,
                  (const Botan::X509_Certificate& issuer, const Botan::X509_Certificate& subject),
                  (override));
};

class MockOcspProvider : public IOcspProvider {
   public:
      MOCK_METHOD(std::vector<std::uint8_t>,
                  get_ocsp_response,
                  (const Botan::X509_Certificate& issuer, const Botan::X509_Certificate& subject),
                  (override));
};

/// A response that fetch() can hand out while the test keeps observing the same mock.
/// The unique_ptr given to the cache forwards every call to the shared mock.
class ForwardingOcspResponse final : public IOcspResponse {
   public:
      explicit ForwardingOcspResponse(MockOcspResponse& target) : target_(target) {}

      bool successful() const override { return target_.successful(); }

      Botan::Certificate_Status_Code status_for(const Botan::X509_Certificate& issuer,
                                                const Botan::X509_Certificate& subject,
                                                std::chrono::system_clock::time_point ref_time) const override {
         return target_.status_for(issuer, subject, ref_time);
      }

      std::vector<std::uint8_t> raw_bits() const override { return target_.raw_bits(); }

   private:
      MockOcspResponse& target_;
};
