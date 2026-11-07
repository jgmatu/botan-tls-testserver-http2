#include "http/RequestTarget.hpp"

#include <gtest/gtest.h>

TEST(RequestTargetTest, RequestPathDropsQueryAndFragment) {
   EXPECT_EQ(request_path("/a/b?x=1"), "/a/b");
   EXPECT_EQ(request_path("/a/b#top"), "/a/b");
   EXPECT_EQ(request_path("/a?x=1#y"), "/a");
   EXPECT_EQ(request_path("/plain"), "/plain");
   EXPECT_EQ(request_path("?only=query"), "");
}

TEST(RequestTargetTest, SafePathsAreAbsoluteWithoutDotDot) {
   EXPECT_TRUE(is_safe_request_path("/"));
   EXPECT_TRUE(is_safe_request_path("/css/site.css"));
   EXPECT_FALSE(is_safe_request_path(""));
   EXPECT_FALSE(is_safe_request_path("relative/path"));
   EXPECT_FALSE(is_safe_request_path("/../etc/passwd"));
   EXPECT_FALSE(is_safe_request_path("/a/..b"));
}

TEST(RequestTargetTest, PathCatAvoidsDoubleSlash) {
   EXPECT_EQ(path_cat("webroot", "/index.html"), "webroot/index.html");
   EXPECT_EQ(path_cat("webroot/", "/index.html"), "webroot/index.html");
   EXPECT_EQ(path_cat("", "/index.html"), "/index.html");
}

TEST(RequestTargetTest, TrailingSlashMapsToIndexHtml) {
   EXPECT_EQ(resolve_file_path("webroot", "/"), "webroot/index.html");
   EXPECT_EQ(resolve_file_path("webroot", "/docs/"), "webroot/docs/index.html");
   EXPECT_EQ(resolve_file_path("webroot", "/logo.png"), "webroot/logo.png");
}
