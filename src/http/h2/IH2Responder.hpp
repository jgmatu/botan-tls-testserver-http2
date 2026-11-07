#pragma once

#include "http/h2/H2Message.hpp"

#include <string>
#include <vector>

/**
 * Queues one response for the stream that just completed.
 * Valid only for the duration of IH2Application::on_request.
 */
class IH2Responder {
   public:
      virtual ~IH2Responder() = default;

      virtual void submit(int status, std::vector<H2Header> headers, std::string body) = 0;

      virtual bool submitted() const = 0;
};
