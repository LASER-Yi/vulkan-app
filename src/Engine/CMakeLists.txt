project(Engine)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "The engine is not compiled for 32-bit systems.")
endif()

find_package(Vulkan REQUIRED COMPONENTS Vulkan)

include(CompileTarget)
include(PlatformMarcos)

add_library(${PROJECT_NAME} ${TARGET_SOURCES} ${TARGET_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${TARGET_HEADERS_DIR})

if (Vulkan_FOUND)
    target_include_directories(${PROJECT_NAME} INTERFACE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Cannot find Vulkan SDK")
endif()

target_include_directories(${PROJECT_NAME} INTERFACE ${GLFW_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

add_compile_definitions(VULKAN_HPP_NO_CONSTRUCTORS)
add_compile_definitions(VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)
