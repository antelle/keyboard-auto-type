project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME} main.cpp)

if(APPLE)
    set(SYSTEM_LIBRARIES
        "-framework CoreFoundation"
        "-framework Carbon"
        "-framework AppKit"
        "-framework ScriptingBridge"
    )
elseif(WIN32)
    add_definitions(-DUNICODE -D_UNICODE -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    set(SYSTEM_LIBRARIES
        "Shlwapi.lib"
    )
endif()

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

add_custom_target(run-tests
    COMMAND echo TESTS: "${OUTPUT_NAME}"
)
