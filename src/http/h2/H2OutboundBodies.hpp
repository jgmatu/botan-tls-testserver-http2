#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

/// Response bodies waiting to be pulled by the HTTP/2 engine, one per stream.
class H2OutboundBodies {
   public:
      class Body {
         public:
            explicit Body(std::string data) : data_(std::move(data)) {}

            /// Copies up to @p length bytes into @p buf. @p eof is set once the body is exhausted.
            std::size_t read(std::uint8_t* buf, std::size_t length, bool& eof);

            bool empty() const { return data_.empty(); }

         private:
            std::string data_;
            std::size_t offset_ = 0;
      };

      /// The returned pointer stays valid until the stream is removed.
      Body* add(int32_t stream_id, std::string body);

      void remove(int32_t stream_id);

      bool contains(int32_t stream_id) const;

   private:
      std::unordered_map<int32_t, std::unique_ptr<Body>> bodies_;
};
