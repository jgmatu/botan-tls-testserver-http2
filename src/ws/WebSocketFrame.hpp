#pragma once

/**
 * @file
 * @brief RFC 6455 frame parsing (client to server) and encoding (server to client).
 * @ingroup ws
 */

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/// Largest frame payload accepted from a client, in bytes.
constexpr std::size_t kMaxFrameSize = 1024 * 1024;

/// @brief WebSocket opcodes used by this server.
namespace ws_opcode {
constexpr std::uint8_t text = 0x1;   ///< UTF-8 text frame.
constexpr std::uint8_t close = 0x8;  ///< Close control frame.
constexpr std::uint8_t ping = 0x9;   ///< Ping control frame.
constexpr std::uint8_t pong = 0xA;   ///< Pong control frame.
}  // namespace ws_opcode

/// @brief One decoded frame.
struct WsFrame {
      std::uint8_t opcode = 0;  ///< Frame opcode (see ws_opcode).
      std::string payload;      ///< Unmasked payload.
};

/**
 * @brief Incremental parser for masked client frames.
 *
 * Bytes are appended as they arrive and next() returns frames once they are
 * complete. Each frame is returned on its own: fragmented messages are not
 * reassembled and the FIN bit is ignored.
 */
class FrameParser {
   public:
      /**
       * @brief Adds received bytes to the internal buffer.
       * @param bytes Bytes read from the stream.
       */
      void append(std::string_view bytes) { buffer_.append(bytes); }

      /**
       * @brief Extracts the next complete frame.
       * @return The frame with its payload unmasked, or std::nullopt if more bytes are needed.
       * @throws std::runtime_error If RSV bits are set, the frame is not masked, the payload exceeds
       *         kMaxFrameSize, or a control frame is longer than 125 bytes.
       */
      std::optional<WsFrame> next();

   private:
      std::string buffer_;
};

/**
 * @brief Encodes a server-to-client frame: FIN set, never masked.
 * @param opcode  Frame opcode (see ws_opcode).
 * @param payload Frame payload.
 * @return The frame bytes, using the 7-bit, 16-bit or 64-bit length form as needed.
 */
std::vector<std::uint8_t> encode_frame(std::uint8_t opcode, std::string_view payload);
