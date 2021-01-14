cmake_minimum_required(VERSION 3.8)

project(keyboard-auto-type)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/output)
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

add_subdirectory(keyboard-auto-type)

if(KEYBOARD_AUTO_TYPE_WITH_EXAMPLE)
    add_subdirectory(example)
endif()

if(KEYBOARD_AUTO_TYPE_WITH_TESTS)
    add_subdirectory(test)
endif()
