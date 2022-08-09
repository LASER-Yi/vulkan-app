project(app)

include(CompileTarget)

add_executable(${PROJECT_NAME} ${TARGET_SOURCES} ${TARGET_HEADERS})

add_subdirectory(shaders)
add_dependencies(${PROJECT_NAME} shaders)

target_link_libraries(${PROJECT_NAME} Engine)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
