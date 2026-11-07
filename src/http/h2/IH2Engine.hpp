#pragma once

/**
 * @file
 * @brief Byte-level interface of an HTTP/2 connection.
 * @ingroup h2
 */

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief Byte-level view of one HTTP/2 connection: what comes in, what goes out.
 *
 * Separates the protocol state machine (H2Session) from the I/O loop
 * (H2ConnectionRunner), so each can be tested on its own. The engine never
 * touches a socket.
 */
class IH2Engine {
   public:
      virtual ~IH2Engine() = default;

      /**
       * @brief Feeds bytes read from the peer.
       * @param data Received bytes.
       * @param len  Number of bytes in @p data.
       * @throws std::exception When the peer violates the protocol or the application fails.
       */
      virtual void consume(const std::uint8_t* data, std::size_t len) = 0;

      /**
       * @brief Next chunk of bytes to send to the peer.
       * @return The bytes, or an empty vector when there is nothing pending.
       * @throws std::exception When the engine cannot serialise its frames.
       */
      virtual std::vector<std::uint8_t> next_outbound() = 0;

      /**
       * @brief Whether the engine still expects input from the peer.
       * @return False once the connection is shutting down on the read side.
       */
      virtual bool wants_read() const = 0;

      /**
       * @brief Whether the engine has, or may still produce, output.
       * @return False once there is nothing left to send.
       */
      virtual bool wants_write() const = 0;
};
