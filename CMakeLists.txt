cmake_minimum_required(VERSION 3.12)

project(keyboard-auto-type)

if(KEYBOARD_AUTO_TYPE_USE_SANITIZERS AND NOT WIN32)
    add_compile_options("-fsanitize=address,undefined")
    add_link_options("-fsanitize=address,undefined")
endif()

if(MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

if(WIN32)
    add_compile_definitions(UNICODE _UNICODE)
endif()

if(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
    add_compile_definitions(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS=1)
endif()

if(KEYBOARD_AUTO_TYPE_DISABLE_CPP_EXCEPTIONS)
    if(MSVC)
        add_compile_definitions(_HAS_EXCEPTIONS=0)
        add_compile_options("/EHs-c-")
    else()
        add_compile_options("-fno-exceptions")
    endif()
endif()

add_subdirectory(keyboard-auto-type)

if(KEYBOARD_AUTO_TYPE_WITH_EXAMPLE)
    add_subdirectory(example)
endif()

if(KEYBOARD_AUTO_TYPE_WITH_TESTS)
    add_subdirectory(test)
endif()

if(KEYBOARD_AUTO_TYPE_WITH_X11_PRINT_LAYOUT)
    add_subdirectory(x11-print-layout)
endif()

if(KEYBOARD_AUTO_TYPE_WITH_CLANG_FORMAT)
    file(GLOB_RECURSE CLANG_FORMAT_FILES
        ${PROJECT_SOURCE_DIR}/example/*.h
        ${PROJECT_SOURCE_DIR}/example/*.cpp
        ${PROJECT_SOURCE_DIR}/keyboard-auto-type/*.h
        ${PROJECT_SOURCE_DIR}/keyboard-auto-type/*.cpp
        ${PROJECT_SOURCE_DIR}/keyboard-auto-type/*.mm
        ${PROJECT_SOURCE_DIR}/test/*.h
        ${PROJECT_SOURCE_DIR}/test/*.cpp
        ${PROJECT_SOURCE_DIR}/test/*.mm
        ${PROJECT_SOURCE_DIR}/x11-print-layout/*.h
        ${PROJECT_SOURCE_DIR}/x11-print-layout/*.cpp
    )
    add_custom_target(clang-format COMMAND clang-format -i --verbose ${CLANG_FORMAT_FILES})
endif()
