#include "ocsp/OcspValidityEstimator.hpp"

#include "CertificateFixture.hpp"
#include "mocks/MockOcsp.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::chrono_literals;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using Code = Botan::Certificate_Status_Code;

class OcspValidityEstimatorTest : public ::testing::Test {
   protected:
      Botan::X509_Certificate issuer = load_fixture_certificate("ca.pem");
      Botan::X509_Certificate subject = load_fixture_certificate("server.pem");
      std::chrono::system_clock::time_point now = std::chrono::system_clock::time_point{} + 1000h;
      ::testing::NiceMock<MockOcspResponse> response;
};

TEST_F(OcspValidityEstimatorTest, ValidResponseKeepsTheMaximumDuration) {
   ON_CALL(response, status_for(_, _, _)).WillByDefault(Return(Code::OCSP_RESPONSE_GOOD));

   EXPECT_EQ(estimate_cache_duration(response, issuer, subject, now, 360min), 360min);
}

TEST_F(OcspValidityEstimatorTest, DurationIsHalvedUntilTheResponseIsValid) {
   // Checks beyond 100 minutes from now report "not yet valid".
   ON_CALL(response, status_for(_, _, _))
      .WillByDefault(Invoke([&](const auto&, const auto&, std::chrono::system_clock::time_point t) {
         return t - now > 100min ? Code::OCSP_NOT_YET_VALID : Code::OCSP_RESPONSE_GOOD;
      }));

   // 360 -> 180 -> 90 is the first duration inside the window.
   EXPECT_EQ(estimate_cache_duration(response, issuer, subject, now, 360min), 90min);
}

TEST_F(OcspValidityEstimatorTest, GivesUpAtZero) {
   ON_CALL(response, status_for(_, _, _)).WillByDefault(Return(Code::OCSP_NOT_YET_VALID));

   EXPECT_EQ(estimate_cache_duration(response, issuer, subject, now, 360min), 0min);
}

TEST_F(OcspValidityEstimatorTest, OtherStatusesDoNotShortenTheDuration) {
   ON_CALL(response, status_for(_, _, _)).WillByDefault(Return(Code::OCSP_HAS_EXPIRED));

   EXPECT_EQ(estimate_cache_duration(response, issuer, subject, now, 60min), 60min);
}

TEST_F(OcspValidityEstimatorTest, ZeroMaximumNeverQueriesTheResponse) {
   EXPECT_CALL(response, status_for(_, _, _)).Times(0);

   EXPECT_EQ(estimate_cache_duration(response, issuer, subject, now, 0min), 0min);
}
