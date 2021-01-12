cmake_minimum_required(VERSION 3.8)

project(keyboard-auto-type)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/output)
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -checks=*,-modernize-use-trailing-return-type,-fuchsia-*,-readability-implicit-bool-conversion,-cppcoreguidelines-pro-type-reinterpret-cast;
    #-checks=-*,cppcoreguidelines-*,-cppcoreguidelines-pro-type-reinterpret-cast,clang-analyzer-*,clang-analyzer-cplusplus*,google-*,cert-*,modernize-*,-modernize-use-trailing-return-type,performance-*,portability-*,readability-*,-readability-implicit-bool-conversion,misc-*;
    -warnings-as-errors=*;
)

add_subdirectory(keyboard-auto-type)
add_subdirectory(example)
