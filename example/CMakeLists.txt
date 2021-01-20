project(example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    "main.cpp"
)

if(APPLE)
    set(SYSTEM_LIBRARIES
        "-framework CoreFoundation"
        "-framework Carbon"
        "-framework AppKit"
        "-framework ScriptingBridge"
    )
elseif(WIN32)
    add_compile_definitions(UNICODE _UNICODE)
    set(SYSTEM_LIBRARIES
        "Shlwapi.lib"
    )
endif()

target_link_libraries(${PROJECT_NAME}
    keyboard-auto-type
    ${SYSTEM_LIBRARIES}
)
