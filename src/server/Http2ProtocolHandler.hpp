#pragma once

#include "http/IRequestHandler.hpp"
#include "server/ISessionHandler.hpp"

#include <memory>
#include <string>

/// HTTP/2: one WebApplication (API + static files) per connection, driven by H2Session.
class Http2ProtocolHandler final : public IProtocolHandler {
   public:
      Http2ProtocolHandler(std::shared_ptr<IRequestHandler> static_files, std::string server);

      boost::asio::awaitable<void> serve(TlsStream& stream,
                                         std::shared_ptr<const IConnectionDetailsProvider> details) override;

   private:
      std::shared_ptr<IRequestHandler> static_files_;
      std::string server_;
};
