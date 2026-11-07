#include "http/h2/H2RequestAssembler.hpp"

#include <string>
#include <utility>

void H2RequestAssembler::begin_stream(int32_t stream_id) {
   auto [it, inserted] = streams_.try_emplace(stream_id);
   if(inserted) {
      it->second.request.stream_id = stream_id;
   }
}

bool H2RequestAssembler::has_stream(int32_t stream_id) const {
   return streams_.contains(stream_id);
}

void H2RequestAssembler::add_header(int32_t stream_id, std::string_view name, std::string_view value) {
   const auto it = streams_.find(stream_id);
   if(it == streams_.end()) {
      return;
   }

   H2Request& request = it->second.request;
   if(name == ":method") {
      request.method = std::string(value);
   } else if(name == ":path") {
      request.path = std::string(value);
   } else if(name.empty() || name.front() != ':') {
      request.headers.push_back(H2Header{std::string(name), std::string(value)});
   }
}

void H2RequestAssembler::add_body(int32_t stream_id, const char* data, std::size_t len) {
   const auto it = streams_.find(stream_id);
   if(it == streams_.end()) {
      return;
   }
   it->second.request.body.append(data, len);
}

std::optional<H2Request> H2RequestAssembler::complete(int32_t stream_id) {
   const auto it = streams_.find(stream_id);
   if(it == streams_.end() || it->second.completed) {
      return std::nullopt;
   }
   it->second.completed = true;
   return std::move(it->second.request);
}

void H2RequestAssembler::discard(int32_t stream_id) {
   streams_.erase(stream_id);
}
