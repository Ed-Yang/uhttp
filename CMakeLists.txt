cmake_minimum_required(VERSION 3.0.2)

project(uhttp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DEFAULT_STARTUP_PROJECT uhttp)
set(CMAKE_BUILD_TYPE Debug)

# vscode cmake tools
include(CMakeToolsHelpers OPTIONAL)

# include
include_directories("./getopt")

# lib
set(EXT_LIBS getopt)
if(WIN32)
add_definitions("/D_CRT_SECURE_NO_WARNINGS")
set(EXT_LIBS ${EXT_LIBS} "wsock32" "Ws2_32")
endif()

file(GLOB UHTTP_SRC "./src/*.c")

add_subdirectory(getopt)

add_executable(uhttp ${UHTTP_SRC})
target_link_libraries(uhttp ${EXT_LIBS})




