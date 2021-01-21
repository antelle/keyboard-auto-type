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
        "src/impl/darwin/auto-release.h"
        "src/impl/darwin/carbon-helpers.h"
        "src/impl/darwin/key-map.h"
        "src/impl/darwin/native-methods.h"
        "src/impl/darwin/auto-type.cpp"
        "src/impl/darwin/carbon-helpers.cpp"
        "src/impl/darwin/key-map.cpp"
        "src/impl/darwin/native-methods.mm"
    )
    source_group("darwin" FILES ${PLATFORM_SOURCES})
elseif(WIN32)
    set(PLATFORM_SOURCES
        "src/impl/win32/key-map.h"
        "src/impl/win32/winapi-tools.h"
        "src/impl/win32/auto-type.cpp"
        "src/impl/win32/key-map.cpp"
        "src/impl/win32/winapi-tools.cpp"
    )
    source_group("win32" FILES ${PLATFORM_SOURCES})
else()
endif()

list(APPEND SOURCES ${PLATFORM_SOURCES})

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        "-framework CoreFoundation"
        "-framework Carbon"
        "-framework AppKit"
        "-framework ScriptingBridge"
    )
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        "Shlwapi.lib"
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
            VS_GLOBAL_ClangTidyChecks "-hicpp-signed-bitwise,-cppcoreguidelines-pro-type-union-access,-llvm-qualified-auto,-readability-qualified-auto"
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
    add_custom_target(cppcheck COMMAND cppcheck --enable=all --inline-suppr --error-exitcode=2 --project=${CPPCHECK_PROJECT_NAME} --suppress=unusedFunction ${SOURCES})
endif()

if(KEYBOARD_AUTO_TYPE_WITH_CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
endif()
