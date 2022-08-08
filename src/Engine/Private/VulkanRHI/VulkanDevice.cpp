#include "VulkanRHI/VulkanDevice.h"

#include "Core/FileManager.h"
#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanGPU.h"
#include "VulkanRHI/VulkanShader.h"
#include "VulkanRHI/VulkanSwapChain.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>

FVulkanDevice::FVulkanDevice(vk::Device device, FVulkanGpu* physicalDevice)
    : physicalDevice(physicalDevice), device(device)
{
    InitRenderPass();
    InitPipeline();

    InitCommandPool();

    InitSwapChain();
    InitDeviceQueue();

    InitFences();
}

FVulkanDevice::~FVulkanDevice()
{
    device.destroyFence(inRenderFence);

    device.destroyCommandPool(commandPool);

    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineLayout(pipelineLayout);

    swapChain.reset();

    device.destroyRenderPass(renderPass);

    shaders.clear();

    device.destroy();
    device = VK_NULL_HANDLE;
}

std::shared_ptr<FVulkanShader>
FVulkanDevice::CreateShader(const std::string& filename,
                            vk::ShaderStageFlagBits stage)
{
    const FileBlob blob = FileManager::ReadFile(filename);

    auto _shader = std::make_shared<FVulkanShader>(this, blob, stage, "main");

    shaders.push_back(_shader);

    return _shader;
}

void FVulkanDevice::Render(vk::CommandBuffer* commandBuffer)
{
    const vk::CommandBufferBeginInfo beginInfo = {
        .sType = vk::StructureType::eCommandBufferBeginInfo,
        .flags = {},
        .pInheritanceInfo = nullptr};

    VERIFY_VULKAN_RESULT(commandBuffer->begin(&beginInfo))

    const std::array<float, 4> defaultClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    vk::ClearValue clearValue = {.color = defaultClearColor};

    const std::vector<vk::ClearValue> clearColors = {
        {.color = {.float32 = {}}}};

    const vk::RenderPassBeginInfo renderPassInfo = {
        .sType = vk::StructureType::eRenderPassBeginInfo,
        .renderPass = renderPass,
        .framebuffer = swapChain->GetFrameBuffer(),
        .renderArea = {.offset = {0, 0}, .extent = swapChain->GetExtent()},
        .clearValueCount = static_cast<uint32_t>(clearColors.size()),
        .pClearValues = clearColors.data(),
    };

    // VK_SUBPASS_CONTENTS_INLINE render pass will be embedded in the primary
    // command buffer

    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS render pass will be
    // executed from secondary command buffer

    commandBuffer->beginRenderPass(&renderPassInfo,
                                   vk::SubpassContents::eInline);

    commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics,
                                graphicsPipeline);

    commandBuffer->setViewport(0, {viewport});
    commandBuffer->setScissor(0, {scissor});

    // DRAW!
    commandBuffer->draw(3, 1, 0, 0);

    commandBuffer->endRenderPass();

    commandBuffer->end();
}

void FVulkanDevice::Submit(vk::CommandBuffer* commandBuffer)
{
    const std::vector<vk::Semaphore> waitSemaphores = {
        GetSwapChain()->GetImageAvailableSemaphore()};

    const std::vector<vk::Semaphore> signalSemaphores = {
        GetSwapChain()->GetRenderFinishedSemaphore()};

    const vk::PipelineStageFlags pipelineFlags = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};

    const vk::SubmitInfo submitInfo = {
        .sType = vk::StructureType::eSubmitInfo,
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = &pipelineFlags,
        .pCommandBuffers = commandBuffer,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data()};

    vk::Queue* graphicsQueue = GetGraphicsQueue();

    graphicsQueue->submit({submitInfo}, inRenderFence);
}

void FVulkanDevice::InitSwapChain()
{
    swapChain = std::make_unique<FVulkanSwapChain>(this);
}

void FVulkanDevice::InitDeviceQueue()
{
    const auto indices = physicalDevice->GetQueueFamilies();

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void FVulkanDevice::InitPipeline()
{
    auto VertShader = CreateShader("shaders/triangle.vert.spv",
                                   vk::ShaderStageFlagBits::eVertex);
    auto FragShader = CreateShader("shaders/triangle.frag.spv",
                                   vk::ShaderStageFlagBits::eFragment);

    const vk::PipelineShaderStageCreateInfo shaderStages[] = {
        VertShader->CreatePipelineStage(), FragShader->CreatePipelineStage()};

    // Dynamic state
    const std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    const vk::PipelineDynamicStateCreateInfo dynamicStateInfo = {
        .sType = vk::StructureType::ePipelineDynamicStateCreateInfo,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    // Vertex input
    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = vk::StructureType::ePipelineVertexInputStateCreateInfo,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    // Input assembly
    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
        .sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo,
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE,
    };

    // Viewports
    const FSwapChainSupportDetails swapChainDetails =
        physicalDevice->GetSwapChainSupportDetails();

    // TODO: Improve API
    const vk::Extent2D extent = swapChainDetails.GetRequiredExtent(nullptr);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset.x = 0.0f;
    scissor.offset.y = 0.0f;
    scissor.extent = extent;

    const vk::PipelineViewportStateCreateInfo viewportState = {
        .sType = vk::StructureType::ePipelineViewportStateCreateInfo,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    // Rasterizer
    const vk::PipelineRasterizationStateCreateInfo rasterizer = {
        .sType = vk::StructureType::ePipelineRasterizationStateCreateInfo,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .lineWidth = 1.0f,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f};

    // Multisampling (required GPU feature)
    const vk::PipelineMultisampleStateCreateInfo multisamplingInfo = {
        .sType = vk::StructureType::ePipelineMultisampleStateCreateInfo,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    // Depth and stencil testing
    // TODO

    // Color blending
    const vk::PipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd};

    const std::array<float, 4> defaultBlendConstants = {0.0f, 0.0f, 0.0f, 0.0f};

    const vk::PipelineColorBlendStateCreateInfo colorBlending = {
        .sType = vk::StructureType::ePipelineColorBlendStateCreateInfo,
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = defaultBlendConstants};

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = vk::StructureType::ePipelineLayoutCreateInfo,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    VERIFY_VULKAN_RESULT(device.createPipelineLayout(&pipelineLayoutInfo,
                                                     nullptr, &pipelineLayout));

    const vk::GraphicsPipelineCreateInfo pipelineInfo = {
        .sType = vk::StructureType::eGraphicsPipelineCreateInfo,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisamplingInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicStateInfo,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = 0};

    const auto pipelines = VERIFY_VULKAN_RESULT_VALUE(
        device.createGraphicsPipelines(nullptr, {pipelineInfo}, nullptr));

    assert(pipelines.size() == 1);
    graphicsPipeline = pipelines[0];
}

void FVulkanDevice::InitRenderPass()
{
    const FSwapChainSupportDetails swapChainDetails =
        physicalDevice->GetSwapChainSupportDetails();

    // Attachment description
    const vk::AttachmentDescription colorAttachment = {
        .format = swapChainDetails.GetRequiredSurfaceFormat().format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR};

    // Attachment reference
    const vk::AttachmentReference colorAttachmentRef = {
        .attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal};

    // Subpass description
    const vk::SubpassDescription subpass = {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef};

    // Subpass dependency
    const vk::SubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eMemoryRead,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
                         vk::AccessFlagBits::eColorAttachmentWrite};

    // Render pass
    const vk::RenderPassCreateInfo renderPassInfo = {
        .sType = vk::StructureType::eRenderPassCreateInfo,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};

    VERIFY_VULKAN_RESULT(
        device.createRenderPass(&renderPassInfo, nullptr, &renderPass));
}

void FVulkanDevice::InitCommandPool()
{
    const auto indices = physicalDevice->GetQueueFamilies();

    const vk::CommandPoolCreateInfo commandPoolInfo = {
        .sType = vk::StructureType::eCommandPoolCreateInfo,
        .queueFamilyIndex = indices.graphicsFamily.value(),
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer};

    VERIFY_VULKAN_RESULT(
        device.createCommandPool(&commandPoolInfo, nullptr, &commandPool));
}

void FVulkanDevice::BeginNextFrame()
{
    VERIFY_VULKAN_RESULT(device.waitForFences(
        {inRenderFence}, VK_TRUE, std::numeric_limits<uint64_t>::max()));

    device.resetFences({inRenderFence});

    GetSwapChain()->AcquireNextImage();
}

vk::CommandBuffer FVulkanDevice::CreateCommandBuffer()
{
    const vk::CommandBufferAllocateInfo allocInfo = {
        .sType = vk::StructureType::eCommandBufferAllocateInfo,
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};

    // VK_COMMAND_BUFFER_LEVEL_PRIMARY means that the command buffer can be
    // submitted directly

    // VK_COMMAND_BUFFER_LEVEL_SECONDARY means that the
    // command buffer can not by submitted directly, but can be called from
    // primary command buffers

    std::vector<vk::CommandBuffer> commandBuffers =
        device.allocateCommandBuffers({allocInfo});

    assert(commandBuffers.size() == 1);

    return commandBuffers[0];
}

void FVulkanDevice::InitFences()
{
    const vk::FenceCreateInfo fenceInfo = {
        .sType = vk::StructureType::eFenceCreateInfo,
        .flags = vk::FenceCreateFlagBits::eSignaled};

    // VK_FENCE_CREATE_SIGNALED_BIT means that the fence is initially signaled

    inRenderFence = device.createFence({fenceInfo});
}
