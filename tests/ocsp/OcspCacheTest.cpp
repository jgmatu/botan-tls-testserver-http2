#include "ocsp/OcspCache.hpp"

#include "CertificateFixture.hpp"
#include "mocks/MockClock.hpp"
#include "mocks/MockLogger.hpp"
#include "mocks/MockOcsp.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::chrono_literals;
using ::testing::_;
using ::testing::HasSubstr;
using ::testing::NiceMock;
using ::testing::Return;
using Code = Botan::Certificate_Status_Code;
using Bytes = std::vector<std::uint8_t>;

class OcspCacheTest : public ::testing::Test {
   protected:
      OcspCacheTest() {
         ON_CALL(response, successful()).WillByDefault(Return(true));
         ON_CALL(response, status_for(_, _, _)).WillByDefault(Return(Code::OCSP_RESPONSE_GOOD));
         ON_CALL(response, raw_bits()).WillByDefault(Return(Bytes{0xDE, 0xAD}));
      }

      /// fetch() hands out a response that forwards to `response`.
      void fetch_returns_response() {
         ON_CALL(*fetcher, fetch(_, _)).WillByDefault([this](const auto&, const auto&) -> std::unique_ptr<IOcspResponse> {
            return std::make_unique<ForwardingOcspResponse>(response);
         });
      }

      Botan::X509_Certificate issuer = load_fixture_certificate("ca.pem");
      Botan::X509_Certificate subject = load_fixture_certificate("server.pem");

      std::shared_ptr<NiceMock<MockOcspFetcher>> fetcher = std::make_shared<NiceMock<MockOcspFetcher>>();
      std::shared_ptr<FakeClock> clock = std::make_shared<FakeClock>();
      std::shared_ptr<NiceMock<MockLogger>> logger = std::make_shared<NiceMock<MockLogger>>();
      NiceMock<MockOcspResponse> response;
      OcspCache cache{fetcher, clock, logger, 60min};
};

TEST_F(OcspCacheTest, FetchesValidatesAndReturnsTheRawResponse) {
   fetch_returns_response();
   EXPECT_CALL(*fetcher, fetch(_, _)).Times(1);

   EXPECT_EQ(cache.get_ocsp_response(issuer, subject), (Bytes{0xDE, 0xAD}));
}

TEST_F(OcspCacheTest, SecondRequestIsServedFromTheCache) {
   fetch_returns_response();
   EXPECT_CALL(*fetcher, fetch(_, _)).Times(1);

   cache.get_ocsp_response(issuer, subject);

   EXPECT_EQ(cache.get_ocsp_response(issuer, subject), (Bytes{0xDE, 0xAD}));
}

TEST_F(OcspCacheTest, CachedResponseExpiresAfterTheConfiguredDuration) {
   fetch_returns_response();
   EXPECT_CALL(*fetcher, fetch(_, _)).Times(2);

   cache.get_ocsp_response(issuer, subject);
   clock->advance(59min);
   cache.get_ocsp_response(issuer, subject);  // still cached
   clock->advance(2min);
   cache.get_ocsp_response(issuer, subject);  // expired, fetched again
}

TEST_F(OcspCacheTest, CachingIsLogged) {
   fetch_returns_response();
   EXPECT_CALL(*logger, info(HasSubstr("caching OCSP response (60min)")));

   cache.get_ocsp_response(issuer, subject);
}

TEST_F(OcspCacheTest, NoResponseFromTheFetcherYieldsNothing) {
   EXPECT_CALL(*fetcher, fetch(_, _)).WillRepeatedly([](const auto&, const auto&) -> std::unique_ptr<IOcspResponse> {
      return nullptr;
   });

   EXPECT_TRUE(cache.get_ocsp_response(issuer, subject).empty());
}

TEST_F(OcspCacheTest, UnsuccessfulResponseIsNotCached) {
   fetch_returns_response();
   ON_CALL(response, successful()).WillByDefault(Return(false));
   EXPECT_CALL(*fetcher, fetch(_, _)).Times(2);

   EXPECT_TRUE(cache.get_ocsp_response(issuer, subject).empty());
   EXPECT_TRUE(cache.get_ocsp_response(issuer, subject).empty());
}

TEST_F(OcspCacheTest, ResponseThatIsNotGoodIsRejected) {
   fetch_returns_response();
   ON_CALL(response, status_for(_, _, _)).WillByDefault(Return(Code::CERT_IS_REVOKED));

   EXPECT_TRUE(cache.get_ocsp_response(issuer, subject).empty());
}

TEST_F(OcspCacheTest, ValidityIsEstimatedWithTheCurrentTime) {
   fetch_returns_response();
   // Not yet valid one hour from now, so the 60 minute budget is halved to 30.
   const auto now = clock->now();
   ON_CALL(response, status_for(_, _, _))
      .WillByDefault([now](const auto&, const auto&, std::chrono::system_clock::time_point t) {
         return t - now >= 60min ? Code::OCSP_NOT_YET_VALID : Code::OCSP_RESPONSE_GOOD;
      });
   EXPECT_CALL(*logger, info(HasSubstr("caching OCSP response (30min)")));

   EXPECT_FALSE(cache.get_ocsp_response(issuer, subject).empty());
}
