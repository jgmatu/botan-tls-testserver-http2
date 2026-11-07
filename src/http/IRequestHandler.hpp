#pragma once

#include "http/h2/H2Message.hpp"
#include "http/h2/IH2Responder.hpp"

/// Answers one family of requests (the REST API, static files, ...).
class IRequestHandler {
   public:
      virtual ~IRequestHandler() = default;

      virtual void handle(const H2Request& request, IH2Responder& respond) = 0;
};
