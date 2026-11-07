#include "http/h2/H2OutboundBodies.hpp"

#include <gtest/gtest.h>

#include <array>
#include <string>

TEST(H2OutboundBodiesTest, ReadsInChunksAndSignalsEofOnTheLastOne) {
   H2OutboundBodies bodies;
   auto* body = bodies.add(1, "abcdefgh");

   std::array<std::uint8_t, 5> buf{};
   bool eof = false;

   EXPECT_EQ(body->read(buf.data(), 5, eof), 5u);
   EXPECT_FALSE(eof);
   EXPECT_EQ(std::string(buf.begin(), buf.begin() + 5), "abcde");

   EXPECT_EQ(body->read(buf.data(), 5, eof), 3u);
   EXPECT_TRUE(eof);
   EXPECT_EQ(std::string(buf.begin(), buf.begin() + 3), "fgh");
}

TEST(H2OutboundBodiesTest, ExhaustedBodyReturnsZeroWithEof) {
   H2OutboundBodies bodies;
   auto* body = bodies.add(1, "a");

   std::array<std::uint8_t, 4> buf{};
   bool eof = false;
   body->read(buf.data(), 4, eof);

   eof = false;
   EXPECT_EQ(body->read(buf.data(), 4, eof), 0u);
   EXPECT_TRUE(eof);
}

TEST(H2OutboundBodiesTest, BodyThatFitsIsFlaggedEofImmediately) {
   H2OutboundBodies bodies;
   auto* body = bodies.add(1, "xyz");

   std::array<std::uint8_t, 16> buf{};
   bool eof = false;
   EXPECT_EQ(body->read(buf.data(), buf.size(), eof), 3u);
   EXPECT_TRUE(eof);
}

TEST(H2OutboundBodiesTest, AddReplacesAndRemoveForgets) {
   H2OutboundBodies bodies;
   bodies.add(1, "old");
   auto* replacement = bodies.add(1, "new body");

   std::array<std::uint8_t, 16> buf{};
   bool eof = false;
   EXPECT_EQ(replacement->read(buf.data(), buf.size(), eof), 8u);
   EXPECT_TRUE(bodies.contains(1));

   bodies.remove(1);
   EXPECT_FALSE(bodies.contains(1));
}
