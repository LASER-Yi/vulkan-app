project(shaders)

include(ShaderCompiler)

add_custom_target(${PROJECT_NAME})

compile_shader(${PROJECT_NAME}
    FORMAT 
        spv
    SOURCES
        triangle.vert
        triangle.frag
)
