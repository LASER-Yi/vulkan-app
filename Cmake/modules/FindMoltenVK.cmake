# Try to find MoltenVK library and include path.
# Once done this will define
#
# MoltenVK_FOUND
# MoltenVK_INCLUDE_DIRS
# MoltenVK_VULKAN_INCLUDE_DIRS
# MoltenVK_LIBRARIES
#

if (NOT APPLE)
    message(FATAL_ERROR "This library is only for Apple systems.")
endif()

find_path( MoltenVK_INCLUDE_DIRS
    NAMES 
        MoltenVK/mvk_vulkan.h
    PATHS
        "$ENV{VULKAN_SDK}/MoltenVK/include"
        "/usr/local/include"
        "/opt/homebrew/include"
    PATH_SUFFIXES
        "MoltenVK"
    DOC
        "MoltenVK library include directory"
)

find_path( MoltenVK_VULKAN_INCLUDE_DIRS
    NAMES 
        vulkan/vulkan.h
    PATHS
        "$ENV{VULKAN_SDK}/MoltenVK/include"
        "/usr/local/include"
        "/opt/homebrew/include"
    PATH_SUFFIXES
        "vk_video"
        "vulkan"
    DOC
        "MoltenVK Vulkan include directory"
)

find_library( MoltenVK_LIBRARIES
    NAMES
        MoltenVK
    HINTS
        # Assume macOS, fix later
        "$ENV{VULKAN_SDK}/MoltenVK/dylib/macOS"
        "/usr/local/lib"
        "/opt/homebrew/lib"
    DOC
        "MoltenVK library"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MoltenVK 
    REQUIRED_VARS
        MoltenVK_INCLUDE_DIRS
        MoltenVK_VULKAN_INCLUDE_DIRS
        MoltenVK_LIBRARIES
)

mark_as_advanced(
    MoltenVK_INCLUDE_DIRS
    MoltenVK_VULKAN_INCLUDE_DIRS
    MoltenVK_LIBRARIES
)
