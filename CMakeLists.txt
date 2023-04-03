cmake_minimum_required(VERSION 3.20)

project(vulkan_app)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/modules"
    )

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Third-party libraries
# GLFW3
include(GLFW)

# add subdirectories
add_subdirectory(src)
