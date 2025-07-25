cmake_minimum_required(VERSION 3.10)
project(AimTrainer C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(extern)
include_directories(extern/raylib/external/glfw/include)

file(GLOB SOURCES "src/*.c")
file(GLOB_RECURSE EXTERNS "extern/*.c")
file(GLOB RAYLIB_PLATFORMS "${CMAKE_SOURCE_DIR}/extern/raylib/platforms/*.c")
file(GLOB_RECURSE RAYLIB_GLFW "${CMAKE_SOURCE_DIR}/extern/raylib/external/glfw/*.c")
list(REMOVE_ITEM EXTERNS
    "${CMAKE_SOURCE_DIR}/extern/clay/clay_renderer_raylib.c"
    "${CMAKE_SOURCE_DIR}/extern/raylib/external/qoaplay.c"
    "${CMAKE_SOURCE_DIR}/extern/raylib/external/stb_vorbis.c"
    ${RAYLIB_PLATFORMS}
    ${RAYLIB_GLFW}

    "${CMAKE_SOURCE_DIR}/extern/lua/lua.c"
    "${CMAKE_SOURCE_DIR}/extern/lua/luac.c"
    "${CMAKE_SOURCE_DIR}/extern/lua/print.c"
)

link_libraries(m)

find_package (CURL REQUIRED)

add_compile_definitions(PLATFORM_DESKTOP_GLFW)
add_executable(aim_trainer ${SOURCES} ${EXTERNS})

target_link_libraries(aim_trainer CURL::libcurl)

message(STATUS "Configuring ${CMAKE_SYSTEM_NAME}...")
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(aim_trainer PRIVATE winmm)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions(_GLFW_X11)
    add_compile_definitions(LUA_USE_POSIX)
endif()

# TODO: eventually should just include the shaders as C strings
add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/src/lighting.vert
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lighting.vert
            &&
            ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/src/lighting.frag
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lighting.frag
            &&
            ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
            &&
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/aim_trainer
    DEPENDS aim_trainer
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running the application..."
)

set(SCRIPT_DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

