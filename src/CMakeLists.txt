cmake_minimum_required(VERSION 3.21)
project(altdumper)

file(GLOB_RECURSE SRC "${PROJECT_SOURCE_DIR}/code_gen/code_gen.cc",
"${PROJECT_SOURCE_DIR}/ptr/ptr.cc",
"${PROJECT_SOURCE_DIR}/ctx/ctx.cc",
"${PROJECT_SOURCE_DIR}/app.cc")
add_executable(${PROJECT_NAME} ${SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
add_definitions(/MP)
add_definitions(/DNOMINMAX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)