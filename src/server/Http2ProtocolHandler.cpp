#include "server/Http2ProtocolHandler.hpp"

#include "http/ApiHandler.hpp"
#include "http/WebApplication.hpp"
#include "http/h2/H2ConnectionRunner.hpp"
#include "http/h2/H2Session.hpp"

Http2ProtocolHandler::Http2ProtocolHandler(std::shared_ptr<IRequestHandler> static_files, std::string server) :
      static_files_(std::move(static_files)), server_(std::move(server)) {}

boost::asio::awaitable<void> Http2ProtocolHandler::serve(TlsStream& stream,
                                                         std::shared_ptr<const IConnectionDetailsProvider> details) {
   WebApplication application(std::make_shared<ApiHandler>(std::move(details), server_), static_files_);
   H2Session session(&application);
   H2ConnectionRunner runner(session);
   co_await runner.run(stream);
}
