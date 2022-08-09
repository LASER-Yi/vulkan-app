
if (CMAKE_BUILD_TYPE STREQUAL "Debug")

    # Only do this on macOS, fix later
    if (APPLE)
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

# Warning as error
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

