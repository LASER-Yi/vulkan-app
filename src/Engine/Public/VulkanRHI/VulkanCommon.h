#pragma once
#include <cstdint>
#include <cstring>
#include <format>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

#define VERIFY_VULKAN_RESULT(VkFunction)                                       \
    {                                                                          \
        const vk::Result ScopedResult = VkFunction;                            \
        if (ScopedResult != vk::Result::eSuccess) {                            \
            VerifyVulkanResult(ScopedResult, #VkFunction, __FILE__, __LINE__); \
        }                                                                      \
    }

#define VERIFY_VULKAN_RESULT_C(VkFunction)                                     \
    {                                                                          \
        const VkResult ScopedResult = VkFunction;                              \
        if (ScopedResult != VK_SUCCESS) {                                      \
            VerifyVulkanResult(ScopedResult, #VkFunction, __FILE__, __LINE__); \
        }                                                                      \
    }

template <class T>
static inline void ZeroVulkanStruct(T& Struct, int32_t VkStructureType)
{
    static_assert(sizeof(T::sType) == sizeof(int32_t),
                  "Assumed sType is compatible with int32!");
    // Horrible way to coerce the compiler to not have to know what T::sType is
    // so we can have this header not have to include vulkan.h
    (int32_t&)Struct.sType = VkStructureType;

    std::memset(((uint8_t*)&Struct) + sizeof(VkStructureType), 0,
                sizeof(T) - sizeof(VkStructureType));
}

template <typename T>
static inline void VerifyVulkanResult(const T& Result, const char* VkFunction,
                                      const char* Filename, uint32_t Line)
{
    throw std::runtime_error(
        std::format("%s failed, VkResult=%d\n at %s:%u \n", VkFunction,
                    static_cast<int32_t>(Result), Filename, Line));
}
