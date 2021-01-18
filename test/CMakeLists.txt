project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        d128fc8252d53baad6ea456fa08cbf9028d255f4
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

if(APPLE)
    set(SYSTEM_LIBRARIES
        "-framework CoreFoundation"
        "-framework Carbon"
        "-framework AppKit"
        "-framework ScriptingBridge"
    )
    set(PLATFORM_SOURCES
        "impl/darwin/platform-util.cpp"
    )
elseif(WIN32)
    add_definitions(-DUNICODE -D_UNICODE -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    set(SYSTEM_LIBRARIES
        "Shlwapi.lib"
    )
    set(PLATFORM_SOURCES
        "impl/win32/platform-util.cpp"
    )
endif()

add_executable(${PROJECT_NAME}
    ${PLATFORM_SOURCES}
    "platform-util.h"
    "main.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC .)

set(SANITIZER_FLAGS "")
if(KEYBOARD_AUTO_TYPE_USE_SANITIZERS AND NOT WIN32)
    set(SANITIZER_FLAGS "-fsanitize=address,undefined")
endif()

target_link_libraries(${PROJECT_NAME}
    keyboard-auto-type
    ${SYSTEM_LIBRARIES}
    gtest
    ${SANITIZER_FLAGS}
)
