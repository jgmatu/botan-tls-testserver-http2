#include "http/MimeType.hpp"

#include <gtest/gtest.h>

#include <utility>

TEST(MimeTypeTest, KnownExtensions) {
   const std::pair<const char*, const char*> cases[] = {
      {"webroot/index.html", "text/html"},
      {"a.htm", "text/html"},
      {"style.css", "text/css"},
      {"notes.txt", "text/plain"},
      {"app.js", "application/javascript"},
      {"data.json", "application/json"},
      {"logo.png", "image/png"},
      {"favicon.ico", "image/png"},
      {"photo.jpg", "image/jpeg"},
      {"photo.jpeg", "image/jpeg"},
      {"photo.jpe", "image/jpeg"},
      {"anim.gif", "image/gif"},
      {"icon.svg", "image/svg+xml"},
      {"icon.svgz", "image/svg+xml"},
   };
   for(const auto& [path, expected] : cases) {
      EXPECT_EQ(mime_type(path), expected) << path;
   }
}

TEST(MimeTypeTest, ExtensionIsCaseInsensitive) {
   EXPECT_EQ(mime_type("INDEX.HTML"), "text/html");
   EXPECT_EQ(mime_type("Logo.PnG"), "image/png");
}

TEST(MimeTypeTest, UnknownOrMissingExtensionFallsBack) {
   EXPECT_EQ(mime_type("archive.tar.gz"), "application/text");
   EXPECT_EQ(mime_type("README"), "application/text");
   EXPECT_EQ(mime_type(""), "application/text");
}
