cmake_minimum_required(VERSION 3.4)
project(server)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  message("using sanitizer")
  set(ASAN_OPTS "-fsanitize=leak")
else()
  set(ASAN_OPTS "")
endif()


conan_cmake_run(
  REQUIRES
    nlohmann_json/3.9.1
    uvw/2.8.0
    spdlog/1.8.2
  BASIC_SETUP CMAKE_TARGETS
  BUILD missing)

add_subdirectory(llhttp)
add_subdirectory(core)
add_subdirectory(api)
