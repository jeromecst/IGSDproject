cmake_minimum_required(VERSION 3.15)

project(ProjetInfoG)
add_executable(ProjetInfoG main5.cpp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

target_link_libraries(ProjetInfoG glfw GLEW GL)

