cmake_minimum_required(VERSION 3.10)
project(http-server)

set(sources
  include/http-server/db/sqlite.hpp
  include/http-server/error.hpp
  include/http-server/http-body-parser.hpp
  include/http-server/http-info.hpp
  include/http-server/http-request-parser.hpp
  include/http-server/http-server.hpp
  include/http-server/log.hpp
  include/http-server/pool.hpp
  include/http-server/pool-worker.hpp
  include/http-server/tcp-client.hpp
  include/http-server/tcp-server.hpp
  include/http-server/utils.hpp
  include/http-server/functor.hpp
  include/http-server/ulid.hpp
  include/http-server/pch.hpp

  src/error.cpp
  src/http-body-parser.cpp
  src/http-info.cpp
  src/http-request-parser.cpp
  src/http-server.cpp
  src/log.cpp
  src/tcp-client.cpp
  src/tcp-server.cpp
  src/utils.cpp
  src/ulid.cpp)

add_library(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC
  sqlite_modern_cpp
  llhttp
  ulid-c
  CONAN_PKG::msgpack
  CONAN_PKG::continuable
  CONAN_PKG::nlohmann_json
  CONAN_PKG::uvw
  CONAN_PKG::spdlog)
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
target_precompile_headers(${PROJECT_NAME} PUBLIC include/http-server/pch.hpp)
set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE
  CXX_EXTENSIONS ON
  LINKER_LANGUAGE CXX)
