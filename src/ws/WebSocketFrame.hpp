#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

constexpr std::size_t kMaxFrameSize = 1024 * 1024;

namespace ws_opcode {
constexpr std::uint8_t text = 0x1;
constexpr std::uint8_t close = 0x8;
constexpr std::uint8_t ping = 0x9;
constexpr std::uint8_t pong = 0xA;
}  // namespace ws_opcode

struct WsFrame {
      std::uint8_t opcode = 0;
      std::string payload;
};

/// Incremental parser for masked client frames. Throws std::runtime_error on protocol violations.
class FrameParser {
   public:
      void append(std::string_view bytes) { buffer_.append(bytes); }

      /// The next complete frame (payload already unmasked), or nothing if more bytes are needed.
      std::optional<WsFrame> next();

   private:
      std::string buffer_;
};

/// Server-to-client frame: FIN set, never masked.
std::vector<std::uint8_t> encode_frame(std::uint8_t opcode, std::string_view payload);
