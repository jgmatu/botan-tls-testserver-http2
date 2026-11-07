#pragma once

#include "http/h2/H2Message.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>

/// Collects the pieces of each request (headers, body chunks) per stream until it is complete.
class H2RequestAssembler {
   public:
      void begin_stream(int32_t stream_id);

      bool has_stream(int32_t stream_id) const;

      /// Pseudo headers :method and :path are kept apart, other pseudo headers are dropped.
      void add_header(int32_t stream_id, std::string_view name, std::string_view value);

      void add_body(int32_t stream_id, const char* data, std::size_t len);

      /// Hands out the request once. Unknown or already completed streams yield nothing.
      std::optional<H2Request> complete(int32_t stream_id);

      void discard(int32_t stream_id);

   private:
      struct StreamState {
            H2Request request;
            bool completed = false;
      };

      std::unordered_map<int32_t, StreamState> streams_;
};
