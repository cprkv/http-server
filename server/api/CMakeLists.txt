cmake_minimum_required(VERSION 3.4)
project(gallery-api)

set(sources src/main.cpp)

add_executable(gallery-api ${sources})
target_include_directories(gallery-api PRIVATE src)
target_link_libraries(gallery-api core)
set_target_properties(gallery-api PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED TRUE
  CXX_EXTENSIONS ON
  LINKER_LANGUAGE CXX)
