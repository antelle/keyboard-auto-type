project(example)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

if(APPLE)
    set(SYSTEM_LIBRARIES
        "-framework CoreFoundation"
        "-framework Carbon"
    )
endif()

target_link_libraries(${PROJECT_NAME}
    keyboard-auto-type
    ${SYSTEM_LIBRARIES}
)
