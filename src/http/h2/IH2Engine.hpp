#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/// Byte-level view of one HTTP/2 connection: what comes in, what goes out.
class IH2Engine {
   public:
      virtual ~IH2Engine() = default;

      /// Feeds bytes read from the peer. Throws when the protocol or the application fails.
      virtual void consume(const std::uint8_t* data, std::size_t len) = 0;

      /// Next chunk of bytes to send to the peer, empty when there is nothing pending.
      virtual std::vector<std::uint8_t> next_outbound() = 0;

      virtual bool wants_read() const = 0;

      virtual bool wants_write() const = 0;
};
