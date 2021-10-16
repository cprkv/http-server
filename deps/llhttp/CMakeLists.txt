cmake_minimum_required(VERSION 3.4)
project(llhttp)

set(sources src/llhttp.c include/llhttp.h src/api.c src/api.h src/http.c)

add_library(llhttp STATIC ${sources})
target_include_directories(llhttp PUBLIC include PRIVATE src)
target_compile_definitions(llhttp PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
set_target_properties(llhttp PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED TRUE
  C_EXTENSIONS ON)
