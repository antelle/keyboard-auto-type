project(example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} "main.cpp")

if(WIN32)
    add_compile_definitions(UNICODE _UNICODE)
endif()

target_link_libraries(${PROJECT_NAME} keyboard-auto-type)
