#include "ocsp/Url.hpp"

#include <gtest/gtest.h>

TEST(UrlTest, HostOnlyGetsRootTargetAndHttpProtocol) {
   const auto url = parse_url("http://ocsp.example.org");

   ASSERT_TRUE(url.has_value());
   EXPECT_EQ(url->host, "ocsp.example.org");
   EXPECT_EQ(url->target, "/");
   EXPECT_EQ(url->protocol, "http");
   EXPECT_FALSE(url->port.has_value());
}

TEST(UrlTest, HostAndPath) {
   const auto url = parse_url("http://ocsp.example.org/status/v1?x=1");

   ASSERT_TRUE(url.has_value());
   EXPECT_EQ(url->host, "ocsp.example.org");
   EXPECT_EQ(url->target, "/status/v1?x=1");
   EXPECT_FALSE(url->port.has_value());
}

TEST(UrlTest, ExplicitPortIsSplitFromTheHost) {
   const auto url = parse_url("http://localhost:8080/ocsp");

   ASSERT_TRUE(url.has_value());
   EXPECT_EQ(url->host, "localhost");
   ASSERT_TRUE(url->port.has_value());
   EXPECT_EQ(*url->port, "8080");
   EXPECT_EQ(url->target, "/ocsp");
}

TEST(UrlTest, PortWithoutPathUsesRootTarget) {
   const auto url = parse_url("http://localhost:8080");

   ASSERT_TRUE(url.has_value());
   EXPECT_EQ(url->host, "localhost");
   EXPECT_EQ(*url->port, "8080");
   EXPECT_EQ(url->target, "/");
}

TEST(UrlTest, WithoutSchemeSeparatorThereIsNoUrl) {
   EXPECT_FALSE(parse_url("ocsp.example.org/path").has_value());
   EXPECT_FALSE(parse_url("").has_value());
}
