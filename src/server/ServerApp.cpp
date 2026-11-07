#include "server/ServerApp.hpp"

#include "common/IClock.hpp"
#include "common/NetErrors.hpp"
#include "http/LocalFileSystem.hpp"
#include "http/StaticFileHandler.hpp"
#include "ocsp/BeastOcspFetcher.hpp"
#include "ocsp/OcspCache.hpp"
#include "server/Http2ProtocolHandler.hpp"
#include "server/TcpListener.hpp"
#include "server/TlsContextFactory.hpp"
#include "server/TlsSessionHandler.hpp"
#include "server/WebSocketProtocolHandler.hpp"

#include <botan/asio_context.h>
#include <botan/version.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <thread>
#include <vector>

namespace {

namespace net = boost::asio;
using tcp = net::ip::tcp;

constexpr std::string_view kAlpnH2 = "h2";
constexpr std::string_view kAlpnHttp11 = "http/1.1";

}  // namespace

ServerApp::ServerApp(ServerOptions options, std::shared_ptr<ILogger> logger) :
      options_(std::move(options)), logger_(std::move(logger)) {}

int ServerApp::run() {
   try {
      auto tls_context = create_tls_context(options_);

      auto clock = std::make_shared<SystemClock>();
      auto fetcher =
         std::make_shared<BeastOcspFetcher>(std::chrono::seconds(options_.ocsp_request_timeout_seconds), logger_);
      auto ocsp_cache =
         std::make_shared<OcspCache>(fetcher, clock, logger_, std::chrono::minutes(options_.ocsp_cache_minutes));

      const std::string server_name = std::string("Botan ") + Botan::short_version_string();
      auto static_files = std::make_shared<StaticFileHandler>(
         options_.document_root, std::make_shared<LocalFileSystem>(), server_name);

      auto websocket = std::make_shared<WebSocketProtocolHandler>();
      auto http2 = std::make_shared<Http2ProtocolHandler>(static_files, server_name);

      // HTTP/2 port: ALPN decides between HTTP/2 and the WebSocket upgrade over HTTP/1.1.
      TlsSessionConfig http_config;
      http_config.tls_context = tls_context;
      http_config.ocsp_provider = ocsp_cache;
      http_config.alpn_preference = {std::string(kAlpnH2), std::string(kAlpnHttp11)};
      http_config.protocols = {{std::string(kAlpnH2), http2}, {std::string(kAlpnHttp11), websocket}};
      // The page connects to its own host:port (ALPN http/1.1 selects the WebSocket), as before the refactoring.
      http_config.websocket_port = options_.port;
      auto http_sessions = std::make_shared<TlsSessionHandler>(std::move(http_config));

      // WebSocket port: always the WebSocket, whatever ALPN was negotiated.
      TlsSessionConfig ws_config;
      ws_config.tls_context = tls_context;
      ws_config.ocsp_provider = ocsp_cache;
      ws_config.alpn_preference = {std::string(kAlpnHttp11)};
      ws_config.protocols = {{std::string(kAlpnHttp11), websocket}};
      ws_config.fallback = websocket;
      ws_config.websocket_port = options_.ws_port;
      auto ws_sessions = std::make_shared<TlsSessionHandler>(std::move(ws_config));

      const auto num_threads = std::thread::hardware_concurrency();
      net::io_context io{static_cast<int>(num_threads)};
      const auto address = net::ip::make_address("0.0.0.0");

      net::io_context ws_io{1};
      net::co_spawn(ws_io,
                    run_tcp_listener(tcp::endpoint{address, options_.ws_port}, ws_sessions, logger_, "WebSocket session"),
                    make_completion_handler(logger_, "WebSocket acceptor"));
      std::jthread ws_thread([&ws_io] { ws_io.run(); });

      struct StopWebSocket {
            net::io_context* io = nullptr;

            ~StopWebSocket() {
               if(io != nullptr) {
                  io->stop();
               }
            }
      } stop_ws{&ws_io};

      net::co_spawn(io,
                    run_tcp_listener(tcp::endpoint{address, options_.port}, http_sessions, logger_, "Session"),
                    make_completion_handler(logger_, "Acceptor"));

      std::vector<std::jthread> threads;
      for(size_t i = 2; i <= num_threads; ++i) {
         threads.emplace_back([&io]() { io.run(); });
      }

      io.run();
   } catch(const std::exception& ex) {
      logger_->error("Server", ex.what());
      return 1;
   }

   return 0;
}
