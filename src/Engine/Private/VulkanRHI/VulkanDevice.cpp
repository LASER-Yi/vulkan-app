#include "VulkanRHI/VulkanDevice.h"

#include "Core/FileManager.h"
#include "VulkanRHI/QueueFamilyIndices.h"
#include "VulkanRHI/SwapChainSupportDetails.h"
#include "VulkanRHI/VulkanCommon.h"
#include "VulkanRHI/VulkanGPU.h"
#include "VulkanRHI/VulkanShader.h"
#include "VulkanRHI/VulkanSwapChain.h"

#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

FVulkanDevice::FVulkanDevice(VkDevice device, FVulkanGpu* physicalDevice)
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
    vkDestroyFence(device, inRenderFence, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    swapChain.reset();

    vkDestroyRenderPass(device, renderPass, nullptr);

    shaders.clear();

    vkDestroyDevice(device, nullptr);
    device = VK_NULL_HANDLE;
}

std::shared_ptr<FVulkanShader>
FVulkanDevice::CreateShader(const std::string& filename,
                            VkShaderStageFlagBits stage)
{
    const FileBlob blob = FileManager::ReadFile(filename);

    auto _shader = std::make_shared<FVulkanShader>(this, blob, stage, "main");

    shaders.push_back(_shader);

    return _shader;
}

void FVulkanDevice::Render(VkCommandBuffer commandBuffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    ZeroVulkanStruct(beginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChain->GetFrameBuffer();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->GetExtent();

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // VK_SUBPASS_CONTENTS_INLINE render pass will be embedded in the primary
    // command buffer

    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS render pass will be
    // executed from secondary command buffer

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // DRAW!
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void FVulkanDevice::Submit(VkCommandBuffer commandBuffer)
{
    VkSubmitInfo submitInfo = {};
    ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore waitSemaphores[] = {
        GetSwapChain()->GetImageAvailableSemaphore()};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;

    const VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    const VkSemaphore signalSemaphores[] = {
        GetSwapChain()->GetRenderFinishedSemaphore()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    const VkQueue graphicsQueue = GetGraphicsQueue();
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inRenderFence) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
}

void FVulkanDevice::InitSwapChain()
{
    assert(device != VK_NULL_HANDLE);

    swapChain = std::make_unique<FVulkanSwapChain>(
        this);
}

void FVulkanDevice::InitDeviceQueue()
{
    assert(device != VK_NULL_HANDLE);

    const auto indices = physicalDevice->GetQueueFamilies();

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void FVulkanDevice::InitPipeline()
{
    auto VertShader =
        CreateShader("shaders/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    auto FragShader =
        CreateShader("shaders/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        VertShader->CreatePipelineStage(), FragShader->CreatePipelineStage()};

    // Dynamic state
    const std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    ZeroVulkanStruct(dynamicStateInfo,
                     VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
    dynamicStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    ZeroVulkanStruct(vertexInputInfo,
                     VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // Viewports
    const FSwapChainSupportDetails swapChainDetails =
        physicalDevice->GetSwapChainSupportDetails();

    // TODO: Improve API
    const VkExtent2D extent = swapChainDetails.GetRequiredExtent(nullptr);

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    ZeroVulkanStruct(viewportState,
                     VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    ZeroVulkanStruct(
        rasterizer, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;

    // Rasterizer: Culling
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // Rasterizer: Depth Bias
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling (required GPU feature)
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    ZeroVulkanStruct(multisampling,
                     VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing
    // TODO

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    ZeroVulkanStruct(colorBlending,
                     VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    const VkResult CreatePipelineResult = vkCreatePipelineLayout(
        device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

    if (CreatePipelineResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    ZeroVulkanStruct(pipelineInfo,
                     VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    // referencing shader stages
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    // referencing pipeline layout
    pipelineInfo.layout = pipelineLayout;

    // render pass
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = 0;

    const VkResult CreateGraphicsPipelineResult = vkCreateGraphicsPipelines(
        device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);

    if (CreateGraphicsPipelineResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
}

void FVulkanDevice::InitRenderPass()
{
    const FSwapChainSupportDetails swapChainDetails =
        physicalDevice->GetSwapChainSupportDetails();

    // Attachment description
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainDetails.GetRequiredSurfaceFormat().format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // Before rendering
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // After rendering
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Attachment reference
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass description
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Render pass
    VkRenderPassCreateInfo renderPassInfo = {};
    ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // Subpass dependency
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    const VkResult CreateRenderPassResult =
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

    if (CreateRenderPassResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}

void FVulkanDevice::InitCommandPool()
{
    const auto indices = physicalDevice->GetQueueFamilies();

    VkCommandPoolCreateInfo commandPoolInfo = {};
    ZeroVulkanStruct(commandPoolInfo,
                     VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    const VkResult CreateCommandPoolResult =
        vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool);

    if (CreateCommandPoolResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

void FVulkanDevice::BeginNextFrame()
{
    vkWaitForFences(device, 1, &inRenderFence, VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &inRenderFence);

    GetSwapChain()->AcquireNextImage();
}

VkCommandBuffer FVulkanDevice::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;

    // VK_COMMAND_BUFFER_LEVEL_PRIMARY means that the command buffer can be
    // submitted directly

    // VK_COMMAND_BUFFER_LEVEL_SECONDARY means that the
    // command buffer can not by submitted directly, but can be called from
    // primary command buffers

    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    const VkResult AllocateCommandBufferResult =
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    if (AllocateCommandBufferResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }

    return commandBuffer;
}

void FVulkanDevice::InitFences()
{
    VkFenceCreateInfo fenceInfo = {};
    ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // VK_FENCE_CREATE_SIGNALED_BIT means that the fence is initially signaled
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(device, &fenceInfo, nullptr, &inRenderFence) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence");
    }
}
