cmake_minimum_required(VERSION 3.20)

project(vulkan_app)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(THRID_PARTY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/modules"
    )

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Third party libraries
# GLFW3
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory(${THRID_PARTY_SOURCE_DIR}/glfw)
set_target_properties(glfw PROPERTIES FOLDER "Thirdparty/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "Thirdparty/GLFW3")

# Projects
add_subdirectory(Engine)
add_subdirectory(app)
