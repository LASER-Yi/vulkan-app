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

#define VERIFY_VULKAN_RESULT_VALUE(VkFunction)                                 \
    [](const auto& ResultValue) {                                              \
        if (ResultValue.result != vk::Result::eSuccess) {                      \
            VerifyVulkanResult(ResultValue.result, #VkFunction, __FILE__,      \
                               __LINE__);                                      \
        }                                                                      \
        return ResultValue.value;                                              \
    }(VkFunction);

template <typename T>
static inline void VerifyVulkanResult(const T& Result, const char* VkFunction,
                                      const char* Filename, uint32_t Line)
{
#if __cpp_lib_format
    throw std::runtime_error(
        std::format("%s failed, VkResult=%d\n at %s:%u \n", VkFunction,
                    static_cast<int32_t>(Result), Filename, Line));
#endif
}
