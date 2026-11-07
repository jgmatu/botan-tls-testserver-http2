#include "http/WebApplication.hpp"

#include "mocks/MockH2Responder.hpp"
#include "mocks/MockRequestHandler.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <stdexcept>

using ::testing::_;
using ::testing::Ref;

class WebApplicationTest : public ::testing::Test {
   protected:
      std::shared_ptr<MockRequestHandler> api = std::make_shared<MockRequestHandler>();
      std::shared_ptr<MockRequestHandler> files = std::make_shared<MockRequestHandler>();
      MockH2Responder responder;
      WebApplication app{api, files};
};

TEST_F(WebApplicationTest, ApiPathsGoToTheApiHandler) {
   const H2Request request = make_request("GET", "/api/connection_details");
   EXPECT_CALL(*api, handle(Ref(request), Ref(responder)));
   EXPECT_CALL(*files, handle(_, _)).Times(0);

   app.on_request(request, responder);
}

TEST_F(WebApplicationTest, ApiPrefixWithQueryStillGoesToTheApiHandler) {
   const H2Request request = make_request("GET", "/api?x=1");
   EXPECT_CALL(*api, handle(_, _));
   EXPECT_CALL(*files, handle(_, _)).Times(0);

   app.on_request(request, responder);
}

TEST_F(WebApplicationTest, EverythingElseGoesToStaticFiles) {
   const H2Request request = make_request("GET", "/index.html");
   EXPECT_CALL(*files, handle(Ref(request), Ref(responder)));
   EXPECT_CALL(*api, handle(_, _)).Times(0);

   app.on_request(request, responder);
}

TEST_F(WebApplicationTest, QueryContainingApiDoesNotCountAsApiPath) {
   const H2Request request = make_request("GET", "/page?next=/api");
   EXPECT_CALL(*files, handle(_, _));
   EXPECT_CALL(*api, handle(_, _)).Times(0);

   app.on_request(request, responder);
}

TEST(WebApplicationConstructionTest, RejectsMissingHandlers) {
   EXPECT_THROW(WebApplication(nullptr, std::make_shared<MockRequestHandler>()), std::invalid_argument);
   EXPECT_THROW(WebApplication(std::make_shared<MockRequestHandler>(), nullptr), std::invalid_argument);
}
