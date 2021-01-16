project(keyboard-auto-type)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    include/keyboard-auto-type.h
    include/key-code.h
    src/auto-type.cpp
    src/modifier.cpp
)

if (APPLE)
    set(PLATFORM_SOURCES
        src/impl/darwin/auto-release.h
        src/impl/darwin/carbon-helpers.h
        src/impl/darwin/key-map.h
        src/impl/darwin/native-methods.h
        src/impl/darwin/auto-type.cpp
        src/impl/darwin/carbon-helpers.cpp
        src/impl/darwin/key-map.cpp
        src/impl/darwin/native-methods.mm
    )
    source_group("darwin" FILES ${PLATFORM_SOURCES})
elseif(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    set(PLATFORM_SOURCES
        src/impl/win32/key-map.h
        src/impl/win32/winapi-tools.h
        src/impl/win32/auto-type.cpp
        src/impl/win32/key-map.cpp
        src/impl/win32/winapi-tools.cpp
    )
    source_group("win32" FILES ${PLATFORM_SOURCES})
else()
endif()

list(APPEND SOURCES ${PLATFORM_SOURCES})

add_library(${PROJECT_NAME} STATIC ${SOURCES})

if(APPLE)
    target_compile_options(${PROJECT_NAME} PUBLIC "-fobjc-arc")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC include)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

add_custom_command(
    OUTPUT keyboard-auto-type-clang-tidy
    COMMAND clang-tidy -p build --quiet ${SOURCES}
)

add_custom_target(clang-tidy DEPENDS keyboard-auto-type-clang-tidy)
