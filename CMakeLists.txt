cmake_minimum_required(VERSION 3.5)

project(keyboard-auto-type)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/output)
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

add_subdirectory(keyboard-auto-type)
add_subdirectory(example)
