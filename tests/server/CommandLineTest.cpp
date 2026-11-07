#include "server/CommandLine.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace {

CommandLineResult parse(std::initializer_list<const char*> args) {
   std::vector<const char*> argv = {"testserver"};
   argv.insert(argv.end(), args.begin(), args.end());
   return parse_command_line(static_cast<int>(argv.size()), argv.data());
}

}  // namespace

TEST(CommandLineTest, RequiredOptionsOnlyUseTheDefaults) {
   const auto result = parse({"--port", "50443", "--cert", "server.pem", "--key", "server.key"});

   ASSERT_EQ(result.status, CommandLineResult::Status::Run) << result.message;
   EXPECT_EQ(result.options.port, 50443);
   EXPECT_EQ(result.options.cert, "server.pem");
   EXPECT_EQ(result.options.key, "server.key");
   EXPECT_EQ(result.options.ws_port, 50444);
   EXPECT_EQ(result.options.policy, "default");
   EXPECT_EQ(result.options.document_root, "webroot");
   EXPECT_EQ(result.options.ocsp_request_timeout_seconds, 10u);
   EXPECT_EQ(result.options.ocsp_cache_minutes, 360u);
}

TEST(CommandLineTest, AllOptionsAreRead) {
   const auto result = parse({"--port", "8443",
                              "--ws-port", "8444",
                              "--policy", "policies/pqc_basic.txt",
                              "--cert", "c.pem",
                              "--key", "k.pem",
                              "--ocsp-request-timeout", "3",
                              "--ocsp-cache-time", "15",
                              "--document-root", "/srv/www"});

   ASSERT_EQ(result.status, CommandLineResult::Status::Run) << result.message;
   EXPECT_EQ(result.options.port, 8443);
   EXPECT_EQ(result.options.ws_port, 8444);
   EXPECT_EQ(result.options.policy, "policies/pqc_basic.txt");
   EXPECT_EQ(result.options.ocsp_request_timeout_seconds, 3u);
   EXPECT_EQ(result.options.ocsp_cache_minutes, 15u);
   EXPECT_EQ(result.options.document_root, "/srv/www");
}

TEST(CommandLineTest, MissingRequiredOptionIsAnError) {
   const auto result = parse({"--port", "50443", "--cert", "server.pem"});

   EXPECT_EQ(result.status, CommandLineResult::Status::Error);
   EXPECT_THAT(result.message, ::testing::HasSubstr("key"));
}

TEST(CommandLineTest, UnknownOptionIsAnError) {
   const auto result = parse({"--bogus"});

   EXPECT_EQ(result.status, CommandLineResult::Status::Error);
   EXPECT_THAT(result.message, ::testing::HasSubstr("bogus"));
}

TEST(CommandLineTest, InvalidPortValueIsAnError) {
   const auto result = parse({"--port", "not-a-number", "--cert", "c", "--key", "k"});

   EXPECT_EQ(result.status, CommandLineResult::Status::Error);
}

TEST(CommandLineTest, HelpListsTheOptions) {
   const auto result = parse({"--help"});

   EXPECT_EQ(result.status, CommandLineResult::Status::Help);
   EXPECT_THAT(result.message, ::testing::StartsWith("Usage:"));
   EXPECT_THAT(result.message, ::testing::HasSubstr("--port"));
   EXPECT_THAT(result.message, ::testing::HasSubstr("--document-root"));
}

TEST(CommandLineTest, NoArgumentsShowsTheUsage) {
   const auto result = parse({});

   EXPECT_EQ(result.status, CommandLineResult::Status::Help);
}
