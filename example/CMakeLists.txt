project(example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} keyboard-auto-type)
