include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY  https://github.com/glfw/glfw.git
    GIT_TAG         3.3.8
    GIT_SHALLOW     TRUE
)

option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)

FetchContent_MakeAvailable(glfw)
set(GLFW_INCLUDE_DIR "${glfw_SOURCE_DIR}/include")
set(GLFW_SRC_DIR "${glfw_SOURCE_DIR}/src")
