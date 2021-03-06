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

add_executable(${PROJECT_NAME}
    "src/main.cpp"
    "src/auto-type-keys-test.cpp"
    "src/auto-type-window-test.cpp"
    "src/auto-type-errors-test.cpp"
    "src/modifier-test.cpp"
    "src/utils/test-util.h"
    "src/utils/test-util.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

if(KEYBOARD_AUTO_TYPE_WITH_CODE_COVERAGE)
    target_link_options(${PROJECT_NAME} PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
endif()

target_link_libraries(${PROJECT_NAME}
    keyboard-auto-type
    gtest
)
