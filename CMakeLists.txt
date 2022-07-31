cmake_minimum_required(VERSION 3.23.0)

project (vulkan_renderer)

set(SOURCES
    src/main.cpp)

add_executable(vulkan_renderer ${SOURCES})

target_include_directories(vulkan_renderer PRIVATE ${PROJECT_SOURCE_DIR}/include)
