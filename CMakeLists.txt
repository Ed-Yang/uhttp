cmake_minimum_required(VERSION 3.0.2)

project(uhttp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DEFAULT_STARTUP_PROJECT uhttp)
set(CMAKE_BUILD_TYPE Debug)

# vscode cmake tools
include(CMakeToolsHelpers OPTIONAL)

if(WIN32)
set(W32_LIBS ${W32_LIBS} "wsock32" "Ws2_32")
endif()

file(GLOB UHTTP_SRC "./*.c")

add_executable(uhttp ${UHTTP_SRC})

target_link_libraries(uhttp ${W32_LIBS})


