#include "http/h2/H2RequestAssembler.hpp"

#include <gtest/gtest.h>

TEST(H2RequestAssemblerTest, CollectsPseudoHeadersHeadersAndBody) {
   H2RequestAssembler assembler;
   assembler.begin_stream(3);
   assembler.add_header(3, ":method", "POST");
   assembler.add_header(3, ":path", "/upload?x=1");
   assembler.add_header(3, ":scheme", "https");
   assembler.add_header(3, ":authority", "localhost");
   assembler.add_header(3, "content-type", "text/plain");
   assembler.add_body(3, "hello ", 6);
   assembler.add_body(3, "world", 5);

   const auto request = assembler.complete(3);

   ASSERT_TRUE(request.has_value());
   EXPECT_EQ(request->stream_id, 3);
   EXPECT_EQ(request->method, "POST");
   EXPECT_EQ(request->path, "/upload?x=1");
   EXPECT_EQ(request->body, "hello world");
   ASSERT_EQ(request->headers.size(), 1u) << "only regular headers are kept";
   EXPECT_EQ(request->headers[0].name, "content-type");
   EXPECT_EQ(request->headers[0].value, "text/plain");
}

TEST(H2RequestAssemblerTest, StreamsAreIndependent) {
   H2RequestAssembler assembler;
   assembler.begin_stream(1);
   assembler.begin_stream(3);
   assembler.add_header(1, ":path", "/one");
   assembler.add_header(3, ":path", "/three");
   assembler.add_body(1, "a", 1);

   EXPECT_EQ(assembler.complete(3)->path, "/three");
   EXPECT_EQ(assembler.complete(1)->path, "/one");
}

TEST(H2RequestAssemblerTest, CompleteHandsOutTheRequestOnlyOnce) {
   H2RequestAssembler assembler;
   assembler.begin_stream(1);

   EXPECT_TRUE(assembler.complete(1).has_value());
   EXPECT_FALSE(assembler.complete(1).has_value());
   EXPECT_TRUE(assembler.has_stream(1)) << "the stream stays known until it is discarded";
}

TEST(H2RequestAssemblerTest, UnknownStreamsAreIgnored) {
   H2RequestAssembler assembler;
   assembler.add_header(9, ":path", "/x");
   assembler.add_body(9, "x", 1);

   EXPECT_FALSE(assembler.has_stream(9));
   EXPECT_FALSE(assembler.complete(9).has_value());
}

TEST(H2RequestAssemblerTest, BeginningAnExistingStreamKeepsItsState) {
   H2RequestAssembler assembler;
   assembler.begin_stream(1);
   assembler.add_header(1, ":path", "/kept");
   assembler.begin_stream(1);

   EXPECT_EQ(assembler.complete(1)->path, "/kept");
}

TEST(H2RequestAssemblerTest, DiscardForgetsTheStream) {
   H2RequestAssembler assembler;
   assembler.begin_stream(1);
   assembler.discard(1);

   EXPECT_FALSE(assembler.has_stream(1));
   EXPECT_FALSE(assembler.complete(1).has_value());
}
