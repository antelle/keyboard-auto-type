project(keyboard-auto-type)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(RUN_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        -checks=*,-modernize-use-trailing-return-type,-fuchsia-*,-readability-implicit-bool-conversion,-cppcoreguidelines-pro-type-reinterpret-cast;
        -warnings-as-errors=*;
    )
endif()

set(SOURCES
    include/keyboard-auto-type.h
    include/key-code.h
    src/auto-type.cpp
    src/modifier.cpp
)

if (APPLE)
    set(PLATFORM_SOURCES
        src/impl/darwin/auto-release.h
        src/impl/darwin/key-map.h
        src/impl/darwin/auto-type.cpp
        src/impl/darwin/key-map.cpp
    )
    source_group("darwin" FILES ${PLATFORM_SOURCES})
elseif(WIN32)
else()
endif()

list(APPEND SOURCES ${PLATFORM_SOURCES})

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

unset(CMAKE_CXX_CLANG_TIDY)
