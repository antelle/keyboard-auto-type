cmake_minimum_required(VERSION 3.8)

project(keyboard-auto-type)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/output)
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

if(KEYBOARD_AUTO_TYPE_USE_SANITIZERS AND NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif()

add_subdirectory(keyboard-auto-type)

if(KEYBOARD_AUTO_TYPE_WITH_EXAMPLE)
    add_subdirectory(example)
endif()

if(KEYBOARD_AUTO_TYPE_WITH_TESTS)
    add_subdirectory(test)
endif()
