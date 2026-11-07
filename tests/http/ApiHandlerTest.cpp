#include "http/ApiHandler.hpp"

#include "mocks/MockConnectionDetailsProvider.hpp"
#include "mocks/MockH2Responder.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

TEST(ApiHandlerTest, ConnectionDetailsReturnsProviderJson) {
   auto details = std::make_shared<MockConnectionDetailsProvider>();
   EXPECT_CALL(*details, connection_details_json()).WillOnce(Return(R"({"kex_algo": "X25519"})"));
   MockH2Responder responder;
   ApiHandler handler(details, "TestServer");

   EXPECT_CALL(responder,
               submit(200,
                      ::testing::AllOf(HasHeader("content-type", "application/json"),
                                       HasHeader("server", "TestServer")),
                      R"({"kex_algo": "X25519"})"));

   handler.handle(make_request("GET", "/api/connection_details"), responder);
}

TEST(ApiHandlerTest, QueryStringIsIgnoredWhenRouting) {
   auto details = std::make_shared<MockConnectionDetailsProvider>();
   EXPECT_CALL(*details, connection_details_json()).WillOnce(Return("{}"));
   MockH2Responder responder;
   ApiHandler handler(details, "TestServer");

   EXPECT_CALL(responder, submit(200, _, "{}"));

   handler.handle(make_request("GET", "/api/connection_details?cache=no"), responder);
}

TEST(ApiHandlerTest, MissingProviderYieldsEmptyObject) {
   MockH2Responder responder;
   ApiHandler handler(nullptr, "TestServer");

   EXPECT_CALL(responder, submit(200, _, "{}"));

   handler.handle(make_request("GET", "/api/connection_details"), responder);
}

TEST(ApiHandlerTest, NonGetMethodIs400) {
   auto details = std::make_shared<MockConnectionDetailsProvider>();
   EXPECT_CALL(*details, connection_details_json()).Times(0);
   MockH2Responder responder;
   ApiHandler handler(details, "TestServer");

   EXPECT_CALL(responder, submit(400, _, "Unknown API method"));

   handler.handle(make_request("POST", "/api/connection_details"), responder);
}

TEST(ApiHandlerTest, UnknownEndpointIs404WithPath) {
   auto details = std::make_shared<MockConnectionDetailsProvider>();
   MockH2Responder responder;
   ApiHandler handler(details, "TestServer");

   EXPECT_CALL(responder, submit(404, _, "/api/nothing"));

   handler.handle(make_request("GET", "/api/nothing"), responder);
}
