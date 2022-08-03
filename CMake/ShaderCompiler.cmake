find_package(Vulkan REQUIRED COMPONENTS Vulkan)
find_program(glslc_executable
    NAMES 
        glslc 
    HINTS 
        Vulkan::glslc)

message(STATUS "Compiling shaders using ${glslc_executable}")

set(SHADER_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/out)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

# Compile and copy the shader to runtime directory
function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT" "SOURCES")
    foreach(source ${arg_SOURCES})
        set(SHADER_OUT_RESULT "${SHADER_OUT_DIR}/${source}.${arg_FORMAT}")
        set(SHADER_DEP_RESULT "${SHADER_OUT_DIR}/${source}.d")

        add_custom_command(
            OUTPUT ${SHADER_OUT_RESULT}
            DEPENDS ${source}
            DEPFILE ${SHADER_DEP_RESULT}
            COMMAND
                ${glslc_executable}
                -MD -MF ${SHADER_DEP_RESULT}
                -o ${SHADER_OUT_RESULT}
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
            COMMENT
                "Building shader ${source}"
        )

        set(SHADER_RUNTIME_RESULT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${source}.${arg_FORMAT}")
        add_custom_command(
            POST_BUILD
            OUTPUT ${SHADER_RUNTIME_RESULT}
            DEPENDS ${SHADER_OUT_RESULT}
            COMMAND
                ${CMAKE_COMMAND} -E copy
                ${SHADER_OUT_RESULT}
                ${SHADER_RUNTIME_RESULT}
        )
        target_sources(${target} PRIVATE ${SHADER_RUNTIME_RESULT})
    endforeach()
endfunction()
