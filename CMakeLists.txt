cmake_minimum_required(VERSION 3.10)
project(http-server-project)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(
  REQUIRES
    nlohmann_json/3.10.3
    uvw/2.10.0
    spdlog/1.9.2
    sqlite3/3.36.0
    continuable/4.1.0
    msgpack/3.3.0
  BASIC_SETUP CMAKE_TARGETS
  BUILD missing)

add_subdirectory(deps/llhttp)
add_subdirectory(deps/sqlite_modern_cpp)
add_subdirectory(deps/ulid-c)

add_subdirectory(http-server)
add_subdirectory(example)
