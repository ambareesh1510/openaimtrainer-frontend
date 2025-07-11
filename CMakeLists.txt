cmake_minimum_required(VERSION 3.10)
project(AimTrainer C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(extern)
include_directories(extern/raylib/external/glfw/include)

file(GLOB SOURCES "src/*.c")
file(GLOB_RECURSE EXTERNS "extern/*.c")
file(GLOB RAYLIB_PLATFORMS "${CMAKE_SOURCE_DIR}/extern/raylib/platforms/*.c")
message(STATUS "My list: ${RAYLIB_PLATFORMS}")
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
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LUA REQUIRED lua5.4)

# include_directories(${LUA_INCLUDE_DIRS})
# link_libraries(${LUA_LIBRARIES})
link_libraries(m)

# add_compile_definitions(_GLFW_WAYLAND)
add_compile_definitions(PLATFORM_DESKTOP_GLFW)
add_executable(aim_trainer ${SOURCES} ${EXTERNS})

target_link_libraries(aim_trainer PRIVATE ${PRIVATE_PLATFORM_SPECIFIC_LIBRARIES})
target_link_libraries(aim_trainer PUBLIC ${PUBLIC_PLATFORM_SPECIFIC_LIBRARIES})

# target_link_libraries(aim_trainer ${LUA_LIBRARIES})

# target_link_libraries(aim_trainer raylib)

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
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/aim_trainer
    DEPENDS aim_trainer
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running the application..."
)

set(SCRIPT_DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

