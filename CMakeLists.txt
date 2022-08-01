cmake_minimum_required(VERSION 3.20)

project(vulkan_renderer)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/modules")


add_subdirectory(engine)
add_subdirectory(app)
