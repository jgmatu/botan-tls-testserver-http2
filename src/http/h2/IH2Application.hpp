#pragma once

#include "http/h2/H2Message.hpp"
#include "http/h2/IH2Responder.hpp"

/**
 * Business entry point. on_request runs once per stream, after that
 * stream's request headers and body are complete. The handler does not
 * see frames, flags, or HPACK.
 */
class IH2Application {
   public:
      virtual ~IH2Application() = default;

      virtual void on_request(const H2Request& request, IH2Responder& respond) = 0;
};
