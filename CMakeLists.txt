cmake_minimum_required(VERSION 3.10)
project(AimTrainer C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(extern)

file(GLOB SOURCES "src/*.c")
file(GLOB EXTERNS "extern")
set(EXTERNS
    extern/slotmap/slotmap.c
    extern/slotmap/stack.c
    extern/tomlc17/tomlc17.c
)

find_package(raylib REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LUA REQUIRED lua5.4)

include_directories(${LUA_INCLUDE_DIRS})
link_libraries(${LUA_LIBRARIES})

add_executable(aim_trainer ${SOURCES} ${EXTERNS})

target_link_libraries(aim_trainer ${LUA_LIBRARIES})

target_link_libraries(aim_trainer raylib)

add_custom_target(run
    COMMAND aim_trainer
    DEPENDS aim_trainer
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running the application..."
)

set(SCRIPT_DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_custom_command(
    TARGET aim_trainer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/src/script.lua
            ${SCRIPT_DESTINATION}/script.lua
)

