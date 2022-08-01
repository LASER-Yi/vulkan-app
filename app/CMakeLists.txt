project(app)

include(CompileTarget)

add_executable(${PROJECT_NAME} ${TARGET_SOURCES} ${TARGET_HEADERS})

target_link_libraries(${PROJECT_NAME} engine)
