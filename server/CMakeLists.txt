cmake_minimum_required(VERSION 3.4)
project(server)

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

add_subdirectory(llhttp)
add_subdirectory(core)
add_subdirectory(api)
