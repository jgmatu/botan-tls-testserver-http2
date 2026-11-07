#include "server/TlsSessionHandler.hpp"

#include "common/NetErrors.hpp"
#include "tls/ServerStreamCallbacks.hpp"

#include <botan/tls_session.h>

#include <boost/asio/redirect_error.hpp>

TlsSessionHandler::TlsSessionHandler(TlsSessionConfig config) : config_(std::move(config)) {}

std::shared_ptr<IProtocolHandler> TlsSessionHandler::protocol_for(const std::string& alpn) const {
   const auto it = config_.protocols.find(alpn);
   return it != config_.protocols.end() ? it->second : config_.fallback;
}

boost::asio::awaitable<void> TlsSessionHandler::handle(AwaitableTcp socket) {
   namespace net = boost::asio;
   using tcp = net::ip::tcp;

   auto callbacks = std::make_shared<ServerStreamCallbacks>(
      config_.alpn_preference, config_.ocsp_provider, config_.websocket_port);
   TlsStream tls_stream(std::move(socket), config_.tls_context, callbacks);

   try {
      co_await tls_stream.async_handshake(Botan::TLS::Connection_Side::Server);

      const std::string alpn = tls_stream.native_handle()->application_protocol();
      if(const auto protocol = protocol_for(alpn)) {
         co_await protocol->serve(tls_stream, callbacks);
      }
   } catch(const boost::system::system_error& se) {
      if(!is_benign_disconnect(se.code())) {
         throw;
      }
   }

   boost::system::error_code ec;
   co_await tls_stream.async_shutdown(net::redirect_error(net::use_awaitable, ec));
   tls_stream.next_layer().shutdown(tcp::socket::shutdown_send, ec);
}
