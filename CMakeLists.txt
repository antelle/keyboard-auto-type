cmake_minimum_required(VERSION 3.8)

project(keyboard-auto-type)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/output)
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

if(KEYBOARD_AUTO_TYPE_USE_SANITIZERS AND NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif()

if(MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

add_subdirectory(keyboard-auto-type)

if(KEYBOARD_AUTO_TYPE_WITH_EXAMPLE)
    add_subdirectory(example)
endif()

if(KEYBOARD_AUTO_TYPE_WITH_TESTS)
    add_subdirectory(test)
endif()

file(GLOB_RECURSE CLANG_FORMAT_FILES
    ${PROJECT_SOURCE_DIR}/example/*.h
    ${PROJECT_SOURCE_DIR}/example/*.cpp
    ${PROJECT_SOURCE_DIR}/keyboard-auto-type/*.h
    ${PROJECT_SOURCE_DIR}/keyboard-auto-type/*.cpp
    ${PROJECT_SOURCE_DIR}/keyboard-auto-type/*.mm
    ${PROJECT_SOURCE_DIR}/test/*.h
    ${PROJECT_SOURCE_DIR}/test/*.cpp
    ${PROJECT_SOURCE_DIR}/test/*.mm
)
add_custom_target(clang-format COMMAND clang-format -i --verbose ${CLANG_FORMAT_FILES})
