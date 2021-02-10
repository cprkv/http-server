cmake_minimum_required(VERSION 3.4)
project(ulid-c)

set(sources src/ulid.c src/ulid.h)

add_library(ulid-c STATIC ${sources})
target_include_directories(ulid-c PUBLIC src)
set_target_properties(ulid-c PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED TRUE
    C_EXTENSIONS ON)
