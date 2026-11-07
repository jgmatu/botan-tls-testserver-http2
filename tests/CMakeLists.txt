# Prefer the distribution's GoogleTest (libgtest-dev, libgmock-dev) over stray copies in /usr/local.
find_package(GTest CONFIG QUIET
             HINTS "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/GTest"
             NO_DEFAULT_PATH)
if(NOT GTest_FOUND)
   find_package(GTest REQUIRED)
endif()

include(GoogleTest)

add_executable(${PROJECT_NAME}_tests
               common/StringsTest.cpp
               common/NetErrorsTest.cpp
               http/MimeTypeTest.cpp
               http/RequestTargetTest.cpp
               http/StaticFileHandlerTest.cpp
               http/ApiHandlerTest.cpp
               http/WebApplicationTest.cpp
               http/h2/H2ResponseHeadersTest.cpp
               http/h2/H2RequestAssemblerTest.cpp
               http/h2/H2OutboundBodiesTest.cpp
               http/h2/H2SessionTest.cpp
               http/h2/H2ConnectionRunnerTest.cpp
               ws/WebSocketHandshakeTest.cpp
               ws/WebSocketFrameTest.cpp
               ocsp/UrlTest.cpp
               ocsp/OcspResponseStoreTest.cpp
               ocsp/OcspValidityEstimatorTest.cpp
               ocsp/OcspCacheTest.cpp
               tls/AlpnSelectorTest.cpp
               tls/ConnectionDetailsTest.cpp
               tls/TlsPolicyLoaderTest.cpp
               server/CommandLineTest.cpp)

target_compile_definitions(${PROJECT_NAME}_tests
                           PRIVATE
                             TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

target_include_directories(${PROJECT_NAME}_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}_tests
                      PRIVATE
                        ${PROJECT_NAME}_core
                        GTest::gmock
                        GTest::gtest_main)

gtest_discover_tests(${PROJECT_NAME}_tests)
