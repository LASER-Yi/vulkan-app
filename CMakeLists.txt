cmake_minimum_required(VERSION 3.23)

project (vulkan_renderer)

set(CMAKE_CXX_STANDARD 20)
include_directories("$ENV{HOMEBREW_PREFIX}/include")
link_directories("$ENV{HOMEBREW_PREFIX}/lib")

file(GLOB SOURCES
    src/*.cpp)

file(GLOB HEADERS
    "src/*.h" "include/*.h")

add_executable(vulkan_renderer ${SOURCES})
target_link_libraries(vulkan_renderer PRIVATE MoltenVK glfw vulkan)

target_include_directories(vulkan_renderer PRIVATE ${PROJECT_SOURCE_DIR}/include)
