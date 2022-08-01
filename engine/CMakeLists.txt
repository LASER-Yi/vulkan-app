project(engine)

find_package(Vulkan 1.0 REQUIRED COMPONENTS Vulkan)
find_package(GLFW 3.0.0)


file(GLOB SOURCES
    Private/*.cpp)

file(GLOB HEADERS
    "Public/*.h" "Private/*.h")

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/Public)

if (Vulkan_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Cannot find Vulkan SDK")
endif()

if (GLFW_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARIES})
else()
    message(FATAL_ERROR "Cannot find GLFW")
endif()
