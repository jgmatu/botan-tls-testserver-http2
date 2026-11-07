#include "http/h2/H2ResponseHeaders.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAre;
using ::testing::Pair;

TEST(H2ResponseHeadersTest, StatusComesFirstThenContentLengthThenHeaders) {
   const auto fields = build_response_fields(200, {{"Content-Type", "text/html"}, {"Server", "x"}}, 42);

   EXPECT_THAT(fields,
               ElementsAre(Pair(":status", "200"),
                           Pair("content-length", "42"),
                           Pair("content-type", "text/html"),
                           Pair("server", "x")));
}

TEST(H2ResponseHeadersTest, CallerSuppliedContentLengthIsKept) {
   const auto fields = build_response_fields(200, {{"Content-Length", "1234"}}, 0);

   EXPECT_THAT(fields, ElementsAre(Pair(":status", "200"), Pair("content-length", "1234")));
}

TEST(H2ResponseHeadersTest, EmptyBodyGetsContentLengthZero) {
   const auto fields = build_response_fields(404, {}, 0);

   EXPECT_THAT(fields, ElementsAre(Pair(":status", "404"), Pair("content-length", "0")));
}

TEST(H2ResponseHeadersTest, ContentLengthDetectionIgnoresCase) {
   EXPECT_TRUE(is_content_length("content-length"));
   EXPECT_TRUE(is_content_length("Content-Length"));
   EXPECT_FALSE(is_content_length("content-type"));
}
