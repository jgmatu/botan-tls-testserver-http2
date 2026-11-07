#pragma once

/**
 * @file
 * @brief Per-stream accumulation of incoming HTTP/2 requests.
 * @ingroup h2
 */

#include "http/h2/H2Message.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>

/**
 * @brief Collects the pieces of each request (headers, body chunks) per stream until it is complete.
 *
 * H2Session feeds it from the nghttp2 callbacks: begin_stream() on a new
 * HEADERS frame, add_header() per header, add_body() per DATA chunk,
 * complete() on END_STREAM and discard() when the stream closes. Calls for a
 * stream that was never begun are ignored. Not thread-safe; one instance
 * belongs to one session.
 */
class H2RequestAssembler {
   public:
      /**
       * @brief Starts tracking a stream. Does nothing if the stream is already known.
       * @param stream_id HTTP/2 stream id.
       */
      void begin_stream(int32_t stream_id);

      /**
       * @brief Whether the stream is being tracked.
       * @param stream_id HTTP/2 stream id.
       * @return True between begin_stream() and discard().
       */
      bool has_stream(int32_t stream_id) const;

      /**
       * @brief Records one header.
       *
       * :method and :path go to H2Request::method and H2Request::path. Other
       * pseudo-headers (:scheme, :authority) are dropped.
       *
       * @param stream_id HTTP/2 stream id.
       * @param name      Header name.
       * @param value     Header value.
       */
      void add_header(int32_t stream_id, std::string_view name, std::string_view value);

      /**
       * @brief Appends a DATA chunk to the request body.
       * @param stream_id HTTP/2 stream id.
       * @param data      Chunk bytes.
       * @param len       Number of bytes in @p data.
       */
      void add_body(int32_t stream_id, const char* data, std::size_t len);

      /**
       * @brief Hands out the finished request.
       * @param stream_id HTTP/2 stream id.
       * @return The request the first time; std::nullopt for an unknown or already completed stream.
       */
      std::optional<H2Request> complete(int32_t stream_id);

      /**
       * @brief Forgets the stream.
       * @param stream_id HTTP/2 stream id.
       */
      void discard(int32_t stream_id);

   private:
      struct StreamState {
            H2Request request;
            bool completed = false;
      };

      std::unordered_map<int32_t, StreamState> streams_;
};
