cmake_minimum_required(VERSION 3.4)
project(sqlite_modern_cpp)

add_library(sqlite_modern_cpp INTERFACE)
target_include_directories(sqlite_modern_cpp INTERFACE hdr)
target_link_libraries(sqlite_modern_cpp INTERFACE CONAN_PKG::sqlite3)
