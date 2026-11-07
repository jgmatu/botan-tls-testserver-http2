#include "common/Strings.hpp"

#include <gtest/gtest.h>

TEST(StringsTest, LowerAndUpperCopyChangeOnlyLetters) {
   EXPECT_EQ(lower_copy("Content-Type: X1"), "content-type: x1");
   EXPECT_EQ(upper_copy("get /Index"), "GET /INDEX");
   EXPECT_EQ(lower_copy(""), "");
}

TEST(StringsTest, TrimRemovesHeaderWhitespaceOnly) {
   EXPECT_EQ(trim_copy("  \tvalue \t\r"), "value");
   EXPECT_EQ(trim_copy("a b"), "a b");
   EXPECT_EQ(trim_copy(" \t \r"), "");
   EXPECT_EQ(trim_copy("\rvalue"), "\rvalue") << "leading CR is not header whitespace";
}

TEST(StringsTest, IequalsIgnoresCaseButNotLength) {
   EXPECT_TRUE(iequals("Content-Length", "content-length"));
   EXPECT_TRUE(iequals("", ""));
   EXPECT_FALSE(iequals("abc", "abcd"));
   EXPECT_FALSE(iequals("abc", "abd"));
}
