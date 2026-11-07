#include "http/h2/H2OutboundBodies.hpp"

#include <algorithm>
#include <cstring>

std::size_t H2OutboundBodies::Body::read(std::uint8_t* buf, std::size_t length, bool& eof) {
   const std::size_t left = data_.size() - offset_;
   if(left == 0) {
      eof = true;
      return 0;
   }

   const std::size_t n = std::min(left, length);
   std::memcpy(buf, data_.data() + offset_, n);
   offset_ += n;
   eof = offset_ == data_.size();
   return n;
}

H2OutboundBodies::Body* H2OutboundBodies::add(int32_t stream_id, std::string body) {
   auto& slot = bodies_[stream_id];
   slot = std::make_unique<Body>(std::move(body));
   return slot.get();
}

void H2OutboundBodies::remove(int32_t stream_id) {
   bodies_.erase(stream_id);
}

bool H2OutboundBodies::contains(int32_t stream_id) const {
   return bodies_.contains(stream_id);
}
