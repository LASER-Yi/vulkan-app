project(app)

include(CompileTarget)

add_executable(${PROJECT_NAME} ${TARGET_SOURCES} ${TARGET_HEADERS})

add_subdirectory(shaders)
add_dependencies(${PROJECT_NAME} shaders)

target_link_libraries(${PROJECT_NAME} Engine)
