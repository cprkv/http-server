cmake_minimum_required(VERSION 3.4)
project(gallery-api)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(
  REQUIRES
    nlohmann_json/3.9.1
    uvw/2.8.0
    spdlog/1.8.2
  BASIC_SETUP CMAKE_TARGETS
  BUILD missing)

set(sources src/main.cpp src/http-server.cpp src/http-server.hpp src/tcp-server.cpp src/tcp-server.hpp src/log.cpp src/log.hpp)

add_executable(gallery-api ${sources})
target_include_directories(gallery-api PRIVATE src)
target_link_libraries(gallery-api
  CONAN_PKG::nlohmann_json
  CONAN_PKG::uvw
  CONAN_PKG::spdlog)
target_compile_definitions(gallery-api PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
set_target_properties(gallery-api PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE
  CXX_EXTENSIONS ON
  LINKER_LANGUAGE CXX)
