#include "ws/WebSocketFrame.hpp"

#include <stdexcept>
#include <utility>

std::optional<WsFrame> FrameParser::next() {
   if(buffer_.size() < 2) {
      return std::nullopt;
   }
   const auto b0 = static_cast<std::uint8_t>(buffer_[0]);
   const auto b1 = static_cast<std::uint8_t>(buffer_[1]);
   if((b0 & 0x70) != 0) {
      throw std::runtime_error("websocket RSV bits are set");
   }
   const bool masked = (b1 & 0x80) != 0;
   std::uint64_t length = b1 & 0x7f;
   std::size_t header = 2;
   if(length == 126) {
      header += 2;
      if(buffer_.size() < header) {
         return std::nullopt;
      }
      length = (static_cast<std::uint8_t>(buffer_[2]) << 8) | static_cast<std::uint8_t>(buffer_[3]);
   } else if(length == 127) {
      header += 8;
      if(buffer_.size() < header) {
         return std::nullopt;
      }
      length = 0;
      for(std::size_t i = 0; i < 8; ++i) {
         length = (length << 8) | static_cast<std::uint8_t>(buffer_[2 + i]);
      }
   }
   if(!masked) {
      throw std::runtime_error("websocket client frame is not masked");
   }
   header += 4;
   if(length > kMaxFrameSize) {
      throw std::runtime_error("websocket frame is too large");
   }
   if(buffer_.size() < header + length) {
      return std::nullopt;
   }

   const auto opcode = static_cast<std::uint8_t>(b0 & 0x0f);
   if((opcode & 0x8) != 0 && length > 125) {
      throw std::runtime_error("websocket control frame is too large");
   }

   std::string payload(length, '\0');
   const std::uint8_t mask[4] = {
      static_cast<std::uint8_t>(buffer_[header - 4]),
      static_cast<std::uint8_t>(buffer_[header - 3]),
      static_cast<std::uint8_t>(buffer_[header - 2]),
      static_cast<std::uint8_t>(buffer_[header - 1]),
   };
   for(std::uint64_t i = 0; i < length; ++i) {
      payload[static_cast<std::size_t>(i)] =
         static_cast<char>(static_cast<std::uint8_t>(buffer_[header + i]) ^ mask[i % 4]);
   }
   buffer_.erase(0, header + static_cast<std::size_t>(length));
   return WsFrame{opcode, std::move(payload)};
}

std::vector<std::uint8_t> encode_frame(std::uint8_t opcode, std::string_view payload) {
   std::vector<std::uint8_t> frame;
   frame.reserve(2 + payload.size());
   frame.push_back(static_cast<std::uint8_t>(0x80 | opcode));
   if(payload.size() < 126) {
      frame.push_back(static_cast<std::uint8_t>(payload.size()));
   } else if(payload.size() <= 65535) {
      frame.push_back(126);
      frame.push_back(static_cast<std::uint8_t>((payload.size() >> 8) & 0xff));
      frame.push_back(static_cast<std::uint8_t>(payload.size() & 0xff));
   } else {
      frame.push_back(127);
      for(int shift = 56; shift >= 0; shift -= 8) {
         frame.push_back(static_cast<std::uint8_t>((payload.size() >> shift) & 0xff));
      }
   }
   frame.insert(frame.end(), payload.begin(), payload.end());
   return frame;
}
