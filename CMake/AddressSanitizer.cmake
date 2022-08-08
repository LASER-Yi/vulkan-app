
if (CMAKE_BUILD_TYPE STREQUAL "Debug")

    # Only do this on macOS, fix later
    if (APPLE)
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()
