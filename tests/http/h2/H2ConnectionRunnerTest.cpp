#include "http/h2/H2ConnectionRunner.hpp"

#include "mocks/MockEngine.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <string>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace {

/// Both ends of a loopback TCP connection.
struct SocketPair {
      explicit SocketPair(net::io_context& io) : client(io), server(io) {
         tcp::acceptor acceptor(io, tcp::endpoint(net::ip::make_address("127.0.0.1"), 0));
         client.connect(acceptor.local_endpoint());
         acceptor.accept(server);
      }

      tcp::socket client;
      tcp::socket server;
};

}  // namespace

class H2ConnectionRunnerTest : public ::testing::Test {
   protected:
      /// Runs the runner on the server socket until it returns; reports what it threw.
      std::exception_ptr run_to_completion(H2ConnectionRunner& runner, tcp::socket& server) {
         std::exception_ptr failure;
         bool finished = false;
         net::co_spawn(io, runner.run(server), [&](std::exception_ptr e) {
            failure = e;
            finished = true;
         });
         io.run();
         EXPECT_TRUE(finished);
         return failure;
      }

      net::io_context io;
};

TEST_F(H2ConnectionRunnerTest, WritesEngineOutputAndFeedsReceivedBytesToTheEngine) {
   SocketPair sockets(io);
   ::testing::NiceMock<MockH2Engine> engine;
   ON_CALL(engine, wants_read()).WillByDefault(Return(true));
   ON_CALL(engine, wants_write()).WillByDefault(Return(false));
   EXPECT_CALL(engine, next_outbound())
      .WillOnce(Return(std::vector<std::uint8_t>{'h', 'i'}))
      .WillRepeatedly(Return(std::vector<std::uint8_t>{}));

   std::string received;
   EXPECT_CALL(engine, consume(_, _)).WillRepeatedly(Invoke([&](const std::uint8_t* data, std::size_t len) {
      received.append(reinterpret_cast<const char*>(data), len);
   }));

   net::write(sockets.client, net::buffer(std::string("ping")));
   sockets.client.shutdown(tcp::socket::shutdown_send);

   H2ConnectionRunner runner(engine);
   EXPECT_FALSE(run_to_completion(runner, sockets.server));

   EXPECT_EQ(received, "ping");
   std::string reply(2, '\0');
   net::read(sockets.client, net::buffer(reply));
   EXPECT_EQ(reply, "hi");
}

TEST_F(H2ConnectionRunnerTest, ReturnsWhenTheEngineIsDone) {
   SocketPair sockets(io);
   ::testing::StrictMock<MockH2Engine> engine;
   EXPECT_CALL(engine, wants_read()).WillRepeatedly(Return(false));
   EXPECT_CALL(engine, wants_write()).WillRepeatedly(Return(false));

   H2ConnectionRunner runner(engine);

   EXPECT_FALSE(run_to_completion(runner, sockets.server));
}

TEST_F(H2ConnectionRunnerTest, IdleConnectionIsCancelledAfterTheTimeout) {
   SocketPair sockets(io);
   ::testing::NiceMock<MockH2Engine> engine;
   ON_CALL(engine, wants_read()).WillByDefault(Return(true));
   ON_CALL(engine, wants_write()).WillByDefault(Return(false));
   ON_CALL(engine, next_outbound()).WillByDefault(Return(std::vector<std::uint8_t>{}));
   EXPECT_CALL(engine, consume(_, _)).Times(0);

   H2ConnectionRunner runner(engine, std::chrono::milliseconds(50));

   const auto start = std::chrono::steady_clock::now();
   EXPECT_FALSE(run_to_completion(runner, sockets.server));
   EXPECT_GE(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(40));
}

TEST_F(H2ConnectionRunnerTest, EngineFailureIsPropagated) {
   SocketPair sockets(io);
   ::testing::NiceMock<MockH2Engine> engine;
   ON_CALL(engine, wants_read()).WillByDefault(Return(true));
   ON_CALL(engine, wants_write()).WillByDefault(Return(false));
   ON_CALL(engine, next_outbound()).WillByDefault(Return(std::vector<std::uint8_t>{}));
   EXPECT_CALL(engine, consume(_, _)).WillOnce(::testing::Throw(std::runtime_error("protocol error")));

   net::write(sockets.client, net::buffer(std::string("x")));

   H2ConnectionRunner runner(engine);
   const std::exception_ptr failure = run_to_completion(runner, sockets.server);

   ASSERT_TRUE(failure);
   EXPECT_THROW(std::rethrow_exception(failure), std::runtime_error);
}
