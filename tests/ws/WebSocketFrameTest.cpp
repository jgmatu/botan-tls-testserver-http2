#include "ws/WebSocketFrame.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

namespace {

/// A client frame as a browser would send it: FIN set, masked.
std::string masked_frame(std::uint8_t opcode, const std::string& payload, bool fin = true) {
   const std::uint8_t mask[4] = {0x37, 0xfa, 0x21, 0x3d};
   std::string frame;
   frame.push_back(static_cast<char>((fin ? 0x80 : 0x00) | opcode));
   if(payload.size() < 126) {
      frame.push_back(static_cast<char>(0x80 | payload.size()));
   } else if(payload.size() <= 65535) {
      frame.push_back(static_cast<char>(0x80 | 126));
      frame.push_back(static_cast<char>(payload.size() >> 8));
      frame.push_back(static_cast<char>(payload.size() & 0xff));
   } else {
      frame.push_back(static_cast<char>(0x80 | 127));
      for(int shift = 56; shift >= 0; shift -= 8) {
         frame.push_back(static_cast<char>((static_cast<std::uint64_t>(payload.size()) >> shift) & 0xff));
      }
   }
   for(const auto byte : mask) {
      frame.push_back(static_cast<char>(byte));
   }
   for(std::size_t i = 0; i < payload.size(); ++i) {
      frame.push_back(static_cast<char>(static_cast<std::uint8_t>(payload[i]) ^ mask[i % 4]));
   }
   return frame;
}

}  // namespace

TEST(WebSocketFrameTest, ParsesAMaskedTextFrame) {
   FrameParser parser;
   parser.append(masked_frame(ws_opcode::text, "Hello"));

   const auto frame = parser.next();

   ASSERT_TRUE(frame.has_value());
   EXPECT_EQ(frame->opcode, ws_opcode::text);
   EXPECT_EQ(frame->payload, "Hello");
   EXPECT_FALSE(parser.next().has_value());
}

TEST(WebSocketFrameTest, ParsesTheRfcExampleOfAMaskedHello) {
   // RFC 6455 section 5.7: single-frame masked text message "Hello".
   const std::string bytes("\x81\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58", 11);
   FrameParser parser;
   parser.append(bytes);

   const auto frame = parser.next();

   ASSERT_TRUE(frame.has_value());
   EXPECT_EQ(frame->payload, "Hello");
}

TEST(WebSocketFrameTest, WaitsForMoreBytesWhenTheFrameIsIncomplete) {
   const std::string bytes = masked_frame(ws_opcode::text, "partial frame");
   FrameParser parser;

   parser.append(std::string_view(bytes).substr(0, 1));
   EXPECT_FALSE(parser.next().has_value());
   parser.append(std::string_view(bytes).substr(1, 6));
   EXPECT_FALSE(parser.next().has_value());
   parser.append(std::string_view(bytes).substr(7));

   const auto frame = parser.next();
   ASSERT_TRUE(frame.has_value());
   EXPECT_EQ(frame->payload, "partial frame");
}

TEST(WebSocketFrameTest, ParsesBackToBackFramesOneByOne) {
   FrameParser parser;
   parser.append(masked_frame(ws_opcode::text, "one") + masked_frame(ws_opcode::ping, "two"));

   const auto first = parser.next();
   const auto second = parser.next();

   ASSERT_TRUE(first.has_value());
   ASSERT_TRUE(second.has_value());
   EXPECT_EQ(first->payload, "one");
   EXPECT_EQ(second->opcode, ws_opcode::ping);
   EXPECT_EQ(second->payload, "two");
   EXPECT_FALSE(parser.next().has_value());
}

TEST(WebSocketFrameTest, ParsesSixteenBitLengths) {
   const std::string payload(300, 'a');
   FrameParser parser;
   parser.append(masked_frame(ws_opcode::text, payload));

   const auto frame = parser.next();

   ASSERT_TRUE(frame.has_value());
   EXPECT_EQ(frame->payload, payload);
}

TEST(WebSocketFrameTest, ParsesSixtyFourBitLengths) {
   const std::string payload(70000, 'b');
   FrameParser parser;
   parser.append(masked_frame(ws_opcode::text, payload));

   const auto frame = parser.next();

   ASSERT_TRUE(frame.has_value());
   EXPECT_EQ(frame->payload, payload);
}

TEST(WebSocketFrameTest, ParsesAnEmptyPayload) {
   FrameParser parser;
   parser.append(masked_frame(ws_opcode::ping, ""));

   const auto frame = parser.next();

   ASSERT_TRUE(frame.has_value());
   EXPECT_TRUE(frame->payload.empty());
}

TEST(WebSocketFrameTest, UnmaskedClientFrameIsRejected) {
   FrameParser parser;
   parser.append(std::string("\x81\x05Hello", 7));

   EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(WebSocketFrameTest, ReservedBitsAreRejected) {
   std::string bytes = masked_frame(ws_opcode::text, "x");
   bytes[0] = static_cast<char>(bytes[0] | 0x40);
   FrameParser parser;
   parser.append(bytes);

   EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(WebSocketFrameTest, OversizedControlFrameIsRejected) {
   FrameParser parser;
   parser.append(masked_frame(ws_opcode::ping, std::string(126, 'p')));

   EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(WebSocketFrameTest, FrameAboveTheLimitIsRejectedBeforeItsPayloadArrives) {
   // Only the header announcing 2 MiB is sent; the parser must not wait for the payload.
   std::string header;
   header.push_back(static_cast<char>(0x81));
   header.push_back(static_cast<char>(0x80 | 127));
   const std::uint64_t length = 2 * kMaxFrameSize;
   for(int shift = 56; shift >= 0; shift -= 8) {
      header.push_back(static_cast<char>((length >> shift) & 0xff));
   }
   header.append(4, '\0');
   FrameParser parser;
   parser.append(header);

   EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(WebSocketFrameTest, EncodesShortFramesWithoutMask) {
   const auto frame = encode_frame(ws_opcode::text, "hi");

   EXPECT_THAT(frame, ::testing::ElementsAre(0x81, 0x02, 'h', 'i'));
}

TEST(WebSocketFrameTest, EncodesSixteenBitLength) {
   const std::string payload(200, 'z');

   const auto frame = encode_frame(ws_opcode::text, payload);

   ASSERT_EQ(frame.size(), 4 + payload.size());
   EXPECT_EQ(frame[0], 0x81);
   EXPECT_EQ(frame[1], 126);
   EXPECT_EQ(frame[2], 0);
   EXPECT_EQ(frame[3], 200);
}

TEST(WebSocketFrameTest, EncodesSixtyFourBitLength) {
   const std::string payload(70000, 'z');

   const auto frame = encode_frame(ws_opcode::text, payload);

   ASSERT_EQ(frame.size(), 10 + payload.size());
   EXPECT_EQ(frame[1], 127);
   EXPECT_EQ(frame[7], 0x01);
   EXPECT_EQ(frame[8], 0x11);
   EXPECT_EQ(frame[9], 0x70);  // 70000 = 0x011170
}

TEST(WebSocketFrameTest, EncodedControlOpcodes) {
   EXPECT_EQ(encode_frame(ws_opcode::pong, "")[0], 0x8A);
   EXPECT_EQ(encode_frame(ws_opcode::close, "")[0], 0x88);
}
