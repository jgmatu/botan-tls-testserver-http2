#pragma once

/**
 * @file
 * @brief Storage for response bodies while nghttp2 sends them.
 * @ingroup h2
 */

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief Response bodies waiting to be pulled by the HTTP/2 engine, one per stream.
 *
 * nghttp2 reads a body lazily through a data provider callback, possibly in
 * several DATA frames and after flow control allows it. The body therefore
 * has to outlive the call that submitted the response. H2Session adds it on
 * submit and removes it when the stream closes.
 */
class H2OutboundBodies {
   public:
      /// @brief One response body and how much of it has been sent.
      class Body {
         public:
            /// @param data Complete body.
            explicit Body(std::string data) : data_(std::move(data)) {}

            /**
             * @brief Copies the next part of the body.
             * @param buf    Destination buffer.
             * @param length Capacity of @p buf.
             * @param eof    Set to true once the whole body has been read.
             * @return Number of bytes copied.
             */
            std::size_t read(std::uint8_t* buf, std::size_t length, bool& eof);

            /**
             * @brief Whether the body has no bytes at all.
             * @return True for an empty body, regardless of how much was read.
             */
            bool empty() const { return data_.empty(); }

         private:
            std::string data_;
            std::size_t offset_ = 0;
      };

      /**
       * @brief Stores the body of a stream, replacing any previous one.
       * @param stream_id HTTP/2 stream id.
       * @param body      Body to send.
       * @return Pointer for the nghttp2 data source. Stays valid until remove() for that stream.
       */
      Body* add(int32_t stream_id, std::string body);

      /**
       * @brief Releases the body of a stream, if any.
       * @param stream_id HTTP/2 stream id.
       */
      void remove(int32_t stream_id);

      /**
       * @brief Whether a body is stored for the stream.
       * @param stream_id HTTP/2 stream id.
       * @return True between add() and remove().
       */
      bool contains(int32_t stream_id) const;

   private:
      std::unordered_map<int32_t, std::unique_ptr<Body>> bodies_;
};
