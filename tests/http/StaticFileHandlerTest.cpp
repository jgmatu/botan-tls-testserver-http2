#include "http/StaticFileHandler.hpp"

#include "mocks/MockFileSystem.hpp"
#include "mocks/MockH2Responder.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;

class StaticFileHandlerTest : public ::testing::Test {
   protected:
      std::shared_ptr<MockFileSystem> fs = std::make_shared<MockFileSystem>();
      MockH2Responder responder;
      StaticFileHandler handler{"webroot", fs, "TestServer"};
};

TEST_F(StaticFileHandlerTest, ServesFileWithMimeTypeAndServerHeader) {
   EXPECT_CALL(*fs, type(StrEq("webroot/app.js"), _)).WillOnce(Return(FileType::Regular));
   EXPECT_CALL(*fs, size(StrEq("webroot/app.js"), _)).WillOnce(Return(11));
   EXPECT_CALL(*fs, read_all(StrEq("webroot/app.js"), _))
      .WillOnce(DoAll(SetArgReferee<1>(std::string("hello world")), Return(true)));

   EXPECT_CALL(responder,
               submit(200,
                      ::testing::AllOf(HasHeader("content-type", "application/javascript"),
                                       HasHeader("server", "TestServer")),
                      "hello world"));

   handler.handle(make_request("GET", "/app.js"), responder);
}

TEST_F(StaticFileHandlerTest, DirectoryRequestServesIndexHtml) {
   EXPECT_CALL(*fs, type(StrEq("webroot/index.html"), _)).WillOnce(Return(FileType::Regular));
   EXPECT_CALL(*fs, size(_, _)).WillOnce(Return(4));
   EXPECT_CALL(*fs, read_all(StrEq("webroot/index.html"), _))
      .WillOnce(DoAll(SetArgReferee<1>(std::string("<p/>")), Return(true)));

   EXPECT_CALL(responder, submit(200, HasHeader("content-type", "text/html"), "<p/>"));

   handler.handle(make_request("GET", "/"), responder);
}

TEST_F(StaticFileHandlerTest, QueryStringDoesNotReachTheFileSystem) {
   EXPECT_CALL(*fs, type(StrEq("webroot/app.js"), _)).WillOnce(Return(FileType::NotFound));
   EXPECT_CALL(responder, submit(404, _, "/app.js"));

   handler.handle(make_request("GET", "/app.js?v=3#frag"), responder);
}

TEST_F(StaticFileHandlerTest, HeadReturnsContentLengthWithoutBody) {
   EXPECT_CALL(*fs, type(_, _)).WillOnce(Return(FileType::Regular));
   EXPECT_CALL(*fs, size(_, _)).WillOnce(Return(1234));
   EXPECT_CALL(*fs, read_all(_, _)).Times(0);

   EXPECT_CALL(responder, submit(200, HasHeader("content-length", "1234"), ""));

   handler.handle(make_request("HEAD", "/logo.png"), responder);
}

TEST_F(StaticFileHandlerTest, MethodIsCaseInsensitive) {
   EXPECT_CALL(*fs, type(_, _)).WillOnce(Return(FileType::Regular));
   EXPECT_CALL(*fs, size(_, _)).WillOnce(Return(1));
   EXPECT_CALL(*fs, read_all(_, _)).WillOnce(DoAll(SetArgReferee<1>(std::string("x")), Return(true)));
   EXPECT_CALL(responder, submit(200, _, "x"));

   handler.handle(make_request("get", "/a.txt"), responder);
}

TEST_F(StaticFileHandlerTest, UnknownFileIs404WithThePath) {
   EXPECT_CALL(*fs, type(_, _)).WillOnce(Return(FileType::NotFound));
   EXPECT_CALL(responder, submit(404, _, "/missing.html"));

   handler.handle(make_request("GET", "/missing.html"), responder);
}

TEST_F(StaticFileHandlerTest, OtherMethodsAre400) {
   EXPECT_CALL(*fs, type(_, _)).Times(0);
   EXPECT_CALL(responder, submit(400, _, "Unknown HTTP-method"));

   handler.handle(make_request("POST", "/index.html"), responder);
}

TEST_F(StaticFileHandlerTest, DirectoryTraversalIs400) {
   EXPECT_CALL(*fs, type(_, _)).Times(0);
   EXPECT_CALL(responder, submit(400, _, "Illegal request-target"));

   handler.handle(make_request("GET", "/../secret"), responder);
}

TEST_F(StaticFileHandlerTest, NonRegularFileIs500) {
   EXPECT_CALL(*fs, type(_, _)).WillOnce(Return(FileType::Other));
   EXPECT_CALL(responder, submit(500, _, "An error occured: Not a file"));

   handler.handle(make_request("GET", "/dir"), responder);
}

TEST_F(StaticFileHandlerTest, StatusErrorIs500WithMessage) {
   const auto error = std::make_error_code(std::errc::permission_denied);
   EXPECT_CALL(*fs, type(_, _)).WillOnce(DoAll(SetArgReferee<1>(error), Return(FileType::Other)));
   EXPECT_CALL(responder, submit(500, _, "An error occured: " + error.message()));

   handler.handle(make_request("GET", "/locked"), responder);
}

TEST_F(StaticFileHandlerTest, SizeErrorIs500WithMessage) {
   const auto error = std::make_error_code(std::errc::io_error);
   EXPECT_CALL(*fs, type(_, _)).WillOnce(Return(FileType::Regular));
   EXPECT_CALL(*fs, size(_, _)).WillOnce(DoAll(SetArgReferee<1>(error), Return(0)));
   EXPECT_CALL(responder, submit(500, _, "An error occured: " + error.message()));

   handler.handle(make_request("GET", "/broken"), responder);
}

TEST_F(StaticFileHandlerTest, UnreadableFileIs500) {
   EXPECT_CALL(*fs, type(_, _)).WillOnce(Return(FileType::Regular));
   EXPECT_CALL(*fs, size(_, _)).WillOnce(Return(3));
   EXPECT_CALL(*fs, read_all(_, _)).WillOnce(Return(false));
   EXPECT_CALL(responder, submit(500, _, "An error occured: failed to read file"));

   handler.handle(make_request("GET", "/unreadable"), responder);
}
