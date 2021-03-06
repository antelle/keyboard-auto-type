project(keyboard-auto-type)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    "include/keyboard-auto-type.h"
    "include/key-code.h"
    "src/auto-type.cpp"
    "src/utils.h"
    "src/utils.cpp"
)

if (APPLE)
    set(PLATFORM_SOURCES
        "src/darwin/auto-release.h"
        "src/darwin/carbon-helpers.h"
        "src/darwin/key-map.h"
        "src/darwin/native-methods.h"
        "src/darwin/auto-type-darwin.cpp"
        "src/darwin/carbon-helpers.cpp"
        "src/darwin/key-map.cpp"
        "src/darwin/native-methods.mm"
    )
    source_group("darwin" FILES ${PLATFORM_SOURCES})
elseif(WIN32)
    set(PLATFORM_SOURCES
        "src/win32/key-map.h"
        "src/win32/winapi-tools.h"
        "src/win32/auto-type-win32.cpp"
        "src/win32/key-map.cpp"
        "src/win32/winapi-tools.cpp"
    )
    source_group("win32" FILES ${PLATFORM_SOURCES})
else()
    set(PLATFORM_SOURCES
        "src/linux/atspi-helpers.h"
        "src/linux/key-map.h"
        "src/linux/x11-helpers.h"
        "src/linux/x11-keysym-map.h"
        "src/linux/atspi-helpers.cpp"
        "src/linux/auto-type-linux.cpp"
        "src/linux/key-map.cpp"
        "src/linux/x11-helpers.cpp"
        "src/linux/x11-keysym-map.cpp"
    )
endif()

list(APPEND SOURCES ${PLATFORM_SOURCES})

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if(KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE KEYBOARD_AUTO_TYPE_NO_EXCEPTIONS=1)
endif()

if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        "-framework CoreFoundation"
        "-framework Carbon"
        "-framework AppKit"
        "-framework ScriptingBridge"
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        "Shlwapi.lib"
    )
else()
    include(FindX11)
    if(!X11_FOUND)
        message(FATAL_ERROR "X11 is not found, please install xorg-dev")
    endif()

    find_package(PkgConfig)

    pkg_check_modules(ATSPI atspi-2 REQUIRED)
    pkg_check_modules(XTST xtst REQUIRED)
    pkg_check_modules(GLIB glib-2.0 REQUIRED)
    pkg_check_modules(GOBJECT gobject-2.0 REQUIRED)

    include_directories(
        ${X11_INCLUDE_DIR}
        ${XTST_INCLUDE_DIRS}
        ${ATSPI_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${X11_LIBRARIES}
        ${XTST_LIBRARIES}
        ${ATSPI_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${GOBJECT_LIBRARIES}
    )
endif()

if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

if(KEYBOARD_AUTO_TYPE_WITH_STATIC_ANALYSIS)
    get_target_property(PROJECT_BINARY_DIR ${PROJECT_NAME} BINARY_DIR)

    if(WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_GLOBAL_RunCodeAnalysis true
            VS_GLOBAL_EnableMicrosoftCodeAnalysis true
            VS_GLOBAL_EnableClangTidyCodeAnalysis true
            VS_GLOBAL_ClangTidyChecks "-cppcoreguidelines-pro-type-union-access,-llvm-qualified-auto,-readability-qualified-auto"
        )
        add_custom_target(clang-tidy DEPENDS ${PROJECT_NAME})
    else()
        add_custom_target(clang-tidy COMMAND clang-tidy -p ${PROJECT_BINARY_DIR} ${SOURCES})
    endif()

    if(WIN32)
        set(CPPCHECK_PROJECT_NAME "${PROJECT_BINARY_DIR}/keyboard-auto-type.vcxproj")
    else()
        set(CPPCHECK_PROJECT_NAME "${CMAKE_BINARY_DIR}/compile_commands.json")
    endif()
    add_custom_target(cppcheck COMMAND cppcheck --language=c++ --std=c++17 --enable=all --inline-suppr --error-exitcode=2 --project=${CPPCHECK_PROJECT_NAME} --suppress=unusedFunction --suppress=*:/usr/include/* --suppress=unmatchedSuppression ${SOURCES})
endif()

if(KEYBOARD_AUTO_TYPE_WITH_CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
endif()
