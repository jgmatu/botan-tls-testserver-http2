cmake_minimum_required(VERSION 3.24)

project(testserver)

# Dependencies come from 3rdparty/install.sh, which installs into /usr/local.
set(LOCAL_PREFIX "/usr/local")
list(PREPEND CMAKE_PREFIX_PATH "${LOCAL_PREFIX}")
set(BOOST_ROOT "${LOCAL_PREFIX}")
set(Boost_ROOT "${LOCAL_PREFIX}")

find_package(Botan 3 REQUIRED CONFIG)
if(NOT TARGET Botan::Botan)
   if(TARGET Botan::Botan-static)
      add_library(Botan::Botan ALIAS Botan::Botan-static)
   elseif(TARGET botan::botan-static)
      add_library(Botan::Botan ALIAS botan::botan-static)
   elseif(TARGET botan::botan)
      add_library(Botan::Botan ALIAS botan::botan)
   else()
      message(FATAL_ERROR "Botan ${Botan_VERSION} in ${LOCAL_PREFIX} has no linkable target")
   endif()
endif()

set(_botan_build "${LOCAL_PREFIX}/include/botan-3/botan/build.h")
if(EXISTS "${_botan_build}")
   file(STRINGS "${_botan_build}" _botan_asio REGEX "^#define BOTAN_HAS_BOOST_ASIO ")
   if(NOT _botan_asio)
      set_property(TARGET Botan::Botan APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOTAN_HAS_BOOST_ASIO)
   endif()
endif()

find_package(Boost 1.80.0 CONFIG QUIET COMPONENTS program_options)
if(NOT Boost_FOUND)
   find_package(Boost 1.80.0 MODULE REQUIRED COMPONENTS program_options)
endif()
if(TARGET Boost::headers)
   set(BOOST_HEADERS Boost::headers)
else()
   set(BOOST_HEADERS Boost::boost)
endif()

# The copy in LOCAL_PREFIX wins; the system one (libnghttp2-dev) is the fallback.
find_path(NGHTTP2_INCLUDE_DIR
          NAMES nghttp2/nghttp2.h
          HINTS "${LOCAL_PREFIX}/include"
          REQUIRED)
find_library(NGHTTP2_LIBRARY
             NAMES nghttp2
             HINTS "${LOCAL_PREFIX}/lib" "${LOCAL_PREFIX}/lib64"
             REQUIRED)
add_library(nghttp2::nghttp2 UNKNOWN IMPORTED)
set_target_properties(nghttp2::nghttp2
                      PROPERTIES
                        IMPORTED_LOCATION "${NGHTTP2_LIBRARY}"
                        INTERFACE_INCLUDE_DIRECTORIES "${NGHTTP2_INCLUDE_DIR}")
if(NGHTTP2_LIBRARY MATCHES "\\.a$")
   set_property(TARGET nghttp2::nghttp2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS NGHTTP2_STATICLIB)
endif()

find_package(Threads REQUIRED)

set(CORE_SOURCES
    src/common/Strings.cpp
    src/common/StderrLogger.cpp
    src/common/NetErrors.cpp
    src/http/MimeType.cpp
    src/http/RequestTarget.cpp
    src/http/HttpReply.cpp
    src/http/LocalFileSystem.cpp
    src/http/ApiHandler.cpp
    src/http/StaticFileHandler.cpp
    src/http/WebApplication.cpp
    src/http/h2/H2RequestAssembler.cpp
    src/http/h2/H2ResponseHeaders.cpp
    src/http/h2/H2OutboundBodies.cpp
    src/http/h2/H2Session.cpp
    src/ws/WebSocketHandshake.cpp
    src/ws/WebSocketFrame.cpp
    src/ocsp/Url.cpp
    src/ocsp/BotanOcspResponse.cpp
    src/ocsp/BeastOcspFetcher.cpp
    src/ocsp/OcspResponseStore.cpp
    src/ocsp/OcspValidityEstimator.cpp
    src/ocsp/OcspCache.cpp
    src/tls/BasicCredentialsManager.cpp
    src/tls/TlsPolicyLoader.cpp
    src/tls/AlpnSelector.cpp
    src/tls/ConnectionDetails.cpp
    src/tls/ServerStreamCallbacks.cpp
    src/server/CommandLine.cpp
    src/server/TlsContextFactory.cpp
    src/server/TcpListener.cpp
    src/server/TlsSessionHandler.cpp
    src/server/Http2ProtocolHandler.cpp
    src/server/WebSocketProtocolHandler.cpp
    src/server/ServerApp.cpp)

add_library(${PROJECT_NAME}_core STATIC ${CORE_SOURCES})

target_compile_features(${PROJECT_NAME}_core PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME}_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}_core
                      PUBLIC
                        Botan::Botan
                        ${BOOST_HEADERS}
                        Boost::program_options
                        nghttp2::nghttp2
                        Threads::Threads)

add_executable(${PROJECT_NAME} src/main.cpp)

set_target_properties(${PROJECT_NAME}
                      PROPERTIES
                        CXX_STANDARD            20
                        EXPORT_COMPILE_COMMANDS On)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_core)

option(BUILD_TESTING "Build the unit tests (needs libgtest-dev and libgmock-dev)" ON)
if(BUILD_TESTING)
   enable_testing()
   add_subdirectory(tests)
endif()
