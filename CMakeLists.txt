cmake_minimum_required(VERSION 3.20)

project (vulkan_renderer)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/modules")

find_package(Vulkan 1.0 REQUIRED COMPONENTS Vulkan)
find_package(GLFW 3.0.0)

if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    link_directories(${Vulkan_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Cannot find Vukan SDK")
endif()

if (GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})

else()
    message(FATAL_ERROR "Cannot find GLFW")
endif()

file(GLOB SOURCES
    src/*.cpp)

file(GLOB HEADERS
    "src/*.h" "include/*.h")

add_executable(vulkan_renderer ${SOURCES})
target_link_libraries(vulkan_renderer PRIVATE ${GLFW_LIBRARIES} Vulkan::Vulkan)

target_include_directories(vulkan_renderer PRIVATE ${PROJECT_SOURCE_DIR}/include)
