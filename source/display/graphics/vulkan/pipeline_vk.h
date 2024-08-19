//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#define NOMINMAX
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <limits>
#include <array>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace Vulkan
{

class InteropVK;
class InteropVKCuda;
class InteropVKHost;

//--------------------------------------------------------------
struct PipelineContext
{
    void** bufferData = nullptr;
    VkExtent2D displayExtent = {};
    VkInstance instance = nullptr;
    VkSurfaceKHR surface = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    std::vector<const char*> requiredDeviceExtensions;
    VkExternalMemoryHandleTypeFlagBits externalMemoryHandleType = {};
};

//--------------------------------------------------------------
class PipelineVK
{
public:
    PipelineVK(const Buffer::Config& a_bufferConfig,
               const PipelineContext& a_pipelineContext);
    ~PipelineVK();

    PipelineVK(const PipelineVK&) = delete;
    PipelineVK& operator=(const PipelineVK&) = delete;

    void Render(uint32_t a_displayWidth,
                uint32_t a_displayHeight);

    uint32_t GetSwapChainWidth() const;
    uint32_t GetSwapChainHeight() const;

protected:
    void SelectPhysicalDevice();
    bool TrySelectPhysicalDevice(const VkPhysicalDevice& a_physicalDevice,
                                 const VkSurfaceKHR& a_surface);

    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateFrameBuffers();
    void CreateCommandPool();
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateSharedBuffer();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    // Returns the allocation size that may be different.
    VkDeviceSize CreateBuffer(VkBuffer& a_buffer,
                              VkDeviceMemory& a_bufferMemory,
                              const VkDeviceSize a_bufferSize,
                              const VkBufferUsageFlags a_bufferUsageFlags,
                              const VkMemoryPropertyFlags a_memoryPropertyFlags,
                              const void* a_bufferCreateInfoNext = nullptr,
                              const void* a_memoryAllocateInfoNext = nullptr);

    void CopyBuffer(const VkBuffer& a_sourceBuffer,
                    const VkBuffer& a_destinationBuffer,
                    const VkDeviceSize a_sourceBufferSize);

    void RenderFrame();

private:
    // The number of frames.
    static constexpr uint32_t N = 2;

    // Buffer config, format, and data.
    const Buffer::Config m_bufferConfig;
    const VkFormat m_bufferFormat;
    void** const m_bufferData;

    // Instance and surface.
    const VkInstance m_instance;
    const VkSurfaceKHR m_surface;

    // Physical and logical devices.
    std::vector<const char*> m_requiredExtensions = {};
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;

    // Surface capabilities, format, and present mode.
    VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode;

    // Graphics and present queue family indices.
    uint32_t m_graphicsQueueFamilyIndex;
    uint32_t m_presentQueueFamilyIndex;

    // Graphics and present queues.
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    // Swap chain.
    VkSwapchainKHR m_swapChain;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;

    // Render pass and descriptor set layout.
    VkRenderPass m_renderPass;
    VkDescriptorSetLayout m_descriptorSetLayout;

    // Graphics pipeline.
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    // Command pool and buffers.
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

    // Texture image and memory.
    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;

    // Texture view and sampler.
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;

    // Shared buffer and memory.
    VkBuffer m_sharedBuffer;
    VkDeviceMemory m_sharedBufferMemory;

    // Shared buffer interop helper and handle type.
    friend class InteropVKCuda;
    friend class InteropVKHost;
    std::unique_ptr<InteropVK> m_interopVK = nullptr;
    VkExternalMemoryHandleTypeFlagBits m_externalMemoryHandleType = {};

    // Vertex buffer and memory.
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    // Index buffer, count, and memory.
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    // Descriptor pool and sets.
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    // Synchronization objects.
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrameIndex = 0;

    // Vertices and indices defining the quad to render over the
    // entire display surface. Note the v components are flipped
    // for consistency with the graphics apis where y points up.
    struct Vertex { struct { float x, y; } pos, uv; };
    const std::array<Vertex, 4> QuadVertices =
    { {
        { { -1.0f,-1.0f }, { 0.0f, 1.0f } },
        { { -1.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 1.0f, 1.0f }, { 1.0f, 0.0f } },
        { { 1.0f, -1.0f }, { 1.0f, 1.0f } }
    } };
    const std::array<uint16_t, 6> QuadIndices =
    {
        0, 1, 2, 2, 3, 0
    };
};

} // namespace Vulkan
} // namespace Display
} // namespace Simple

// Inline interop implementations depend on PipelineVK.
#include <display/graphics/vulkan/interop_vk_host.h>
#ifdef CUDA_SUPPORTED
#   include <display/graphics/vulkan/interop_vk_cuda.h>
#endif // CUDA_SUPPORTED

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace Vulkan
{

//--------------------------------------------------------------
constexpr VkFormat GetVkFormat(const Buffer::Format& a_format)
{
    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (a_format)
    {
        case Buffer::Format::RGBA_FLOAT: format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
        case Buffer::Format::RGBA_UINT8: format = VK_FORMAT_R8G8B8A8_UNORM; break;
        case Buffer::Format::RGBA_UINT16: format = VK_FORMAT_R16G16B16A16_UNORM; break;
        default: format = VK_FORMAT_UNDEFINED; break;
    }
    assert(format != VK_FORMAT_UNDEFINED);
    return format;
}

//--------------------------------------------------------------
inline PipelineVK::PipelineVK(const Buffer::Config& a_bufferConfig,
                              const PipelineContext& a_pipelineContext)
    : m_bufferConfig(a_bufferConfig)
    , m_bufferFormat(GetVkFormat(a_bufferConfig.format))
    , m_bufferData(a_pipelineContext.bufferData)
    , m_instance(a_pipelineContext.instance)
    , m_surface(a_pipelineContext.surface)
    , m_requiredExtensions(a_pipelineContext.requiredDeviceExtensions)
    , m_swapChainExtent(a_pipelineContext.displayExtent)
    , m_externalMemoryHandleType(a_pipelineContext.externalMemoryHandleType)
{
    m_requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFrameBuffers();
    CreateCommandPool();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateSharedBuffer();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
}

//--------------------------------------------------------------
inline PipelineVK::~PipelineVK()
{
    // Wait for the device to become idle.
    VULKAN_ENSURE(vkDeviceWaitIdle(m_device));

    for (auto framebuffer : m_swapChainFrameBuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    m_interopVK.reset();
    vkDestroyBuffer(m_device, m_sharedBuffer, nullptr);
    vkFreeMemory(m_device, m_sharedBufferMemory, nullptr);

    vkDestroySampler(m_device, m_textureSampler, nullptr);
    vkDestroyImageView(m_device, m_textureImageView, nullptr);

    vkDestroyImage(m_device, m_textureImage, nullptr);
    vkFreeMemory(m_device, m_textureImageMemory, nullptr);

    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    vkFreeMemory(m_device, m_indexBufferMemory, nullptr);

    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

    for (size_t n = 0; n < N; ++n)
    {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[n], nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[n], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[n], nullptr);
    }

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);
}

//--------------------------------------------------------------
inline void PipelineVK::Render(uint32_t a_displayWidth,
                               uint32_t a_displayHeight)
{
    (void)a_displayWidth;
    (void)a_displayHeight;
    RenderFrame();
}

//--------------------------------------------------------------
inline uint32_t PipelineVK::GetSwapChainWidth() const
{
    return m_swapChainExtent.width;
}

//--------------------------------------------------------------
inline uint32_t PipelineVK::GetSwapChainHeight() const
{
    return m_swapChainExtent.height;
}

//--------------------------------------------------------------
inline void PipelineVK::SelectPhysicalDevice()
{
    // Get the count of physical devices.
    uint32_t deviceCount = 0;
    VULKAN_ENSURE(vkEnumeratePhysicalDevices(m_instance,
                                             &deviceCount,
                                             nullptr));

    // Get the list of physical devices.
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VULKAN_ENSURE(vkEnumeratePhysicalDevices(m_instance,
                                             &deviceCount,
                                             devices.data()));

    // Select the first suitable device.
    for (const auto& device : devices)
    {
        if (TrySelectPhysicalDevice(device, m_surface))
        {
            assert(m_physicalDevice == device);
            break;
        }
    }

    assert(m_physicalDevice != VK_NULL_HANDLE);
}

//--------------------------------------------------------------
inline bool SupportsExtensions(const VkPhysicalDevice& a_physicalDevice,
                               const std::vector<const char*>& a_requiredExtensions)
{
    // Get the device extension count.
    uint32_t extensionCount;
    VULKAN_ENSURE(vkEnumerateDeviceExtensionProperties(a_physicalDevice,
                                                       nullptr,
                                                       &extensionCount,
                                                       nullptr));

    // Get the device extensions.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    VULKAN_ENSURE(vkEnumerateDeviceExtensionProperties(a_physicalDevice,
                                                       nullptr,
                                                       &extensionCount,
                                                       extensions.data()));

    // Find each required extension.
    for (const char* requiredExtension : a_requiredExtensions)
    {
        const auto find = [&requiredExtension](VkExtensionProperties extension)
        {
            return strcmp(extension.extensionName, requiredExtension) == 0;
        };
        const auto foundIt = std::find_if(extensions.cbegin(),
                                          extensions.cend(),
                                          find);
        if (foundIt == extensions.cend())
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------
inline bool GetQueueFamilyIndices(const VkPhysicalDevice& a_physicalDevice,
                                  const VkSurfaceKHR& a_surface,
                                  uint32_t& a_graphicsQueueFamilyIndex,
                                  uint32_t& a_presentQueueFamilyIndex)
{
    // Get the queue family count.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(a_physicalDevice,
                                             &queueFamilyCount,
                                             nullptr);

    // Get the queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(a_physicalDevice,
                                             &queueFamilyCount,
                                             queueFamilies.data());

    // Find the required queue family indices.
    bool graphicsQueueFamilyFound = false;
    bool presentQueueFamilyFound = false;
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            a_graphicsQueueFamilyIndex = i;
            graphicsQueueFamilyFound = true;
        }

        VkBool32 presentSupport = false;
        VULKAN_ENSURE(vkGetPhysicalDeviceSurfaceSupportKHR(a_physicalDevice,
                                                           i,
                                                           a_surface,
                                                           &presentSupport));
        if (presentSupport)
        {
            a_presentQueueFamilyIndex = i;
            presentQueueFamilyFound = true;
        }

        if (graphicsQueueFamilyFound && presentQueueFamilyFound)
        {
            break;
        }
    }

    return graphicsQueueFamilyFound && presentQueueFamilyFound;
}

//--------------------------------------------------------------
inline bool PipelineVK::TrySelectPhysicalDevice(const VkPhysicalDevice& a_physicalDevice,
                                                const VkSurfaceKHR& a_surface)
{
    // Ensure the device supports the required extensions.
    if (!SupportsExtensions(a_physicalDevice,
                            m_requiredExtensions))
    {
        return false;
    }

    uint32_t graphicsQueueFamilyIndex = 0;
    uint32_t presentQueueFamilyIndex = 0;
    if (!GetQueueFamilyIndices(a_physicalDevice,
                               a_surface,
                               graphicsQueueFamilyIndex,
                               presentQueueFamilyIndex))
    {
        return false;
    }

    // Get the number of supported surface formats.
    uint32_t surfaceFormatCount = 0;
    VULKAN_ENSURE(vkGetPhysicalDeviceSurfaceFormatsKHR(a_physicalDevice,
                                                       a_surface,
                                                       &surfaceFormatCount,
                                                       nullptr));
    if (surfaceFormatCount == 0)
    {
        return false;
    }

    // Get the number of supported present modes.
    uint32_t presentModeCount = 0;
    VULKAN_ENSURE(vkGetPhysicalDeviceSurfacePresentModesKHR(a_physicalDevice,
                                                            a_surface,
                                                            &presentModeCount,
                                                            nullptr));
    if (presentModeCount == 0)
    {
        return false;
    }

    // Get the supported surface formats.
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    VULKAN_ENSURE(vkGetPhysicalDeviceSurfaceFormatsKHR(a_physicalDevice,
                                                       a_surface,
                                                       &surfaceFormatCount,
                                                       surfaceFormats.data()));

    // Get the supported present modes.
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VULKAN_ENSURE(vkGetPhysicalDeviceSurfacePresentModesKHR(a_physicalDevice,
                                                            a_surface,
                                                            &presentModeCount,
                                                            presentModes.data()));

    // Select the best available surface format.
    m_surfaceFormat = surfaceFormats[0];
    for (const auto& availableFormat : surfaceFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            m_surfaceFormat = availableFormat;
            break;
        }
    }

    // Select the best available present mode.
    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablePresentMode : presentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            m_presentMode = availablePresentMode;
            break;
        }
    }

    // Get the surface capabilities.
    VULKAN_ENSURE(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(a_physicalDevice,
                                                            a_surface,
                                                            &m_surfaceCapabilities));

    // Set the graphics and present queue family indices.
    m_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    m_presentQueueFamilyIndex = presentQueueFamilyIndex;

    // Set the selected physical device.
    m_physicalDevice = a_physicalDevice;

    return true;
}

//--------------------------------------------------------------
inline void PipelineVK::CreateLogicalDevice()
{
    // Describe the queues.
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const float queuePriority = 1.0f;

    // Describe the graphics queue.
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Describe the present queue.
    if (m_presentQueueFamilyIndex != m_graphicsQueueFamilyIndex)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfo.queueFamilyIndex = m_presentQueueFamilyIndex;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Describe the device.
    VkDeviceCreateInfo createInfo = {};
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = m_requiredExtensions.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // Create the device.
    VULKAN_ENSURE(vkCreateDevice(m_physicalDevice,
                                 &createInfo,
                                 nullptr,
                                 &m_device));

    // Get the graphics queue.
    vkGetDeviceQueue(m_device,
                     m_graphicsQueueFamilyIndex,
                     0,
                     &m_graphicsQueue);

    // Get the present queue.
    vkGetDeviceQueue(m_device,
                     m_presentQueueFamilyIndex,
                     0,
                     &m_presentQueue);
}

//--------------------------------------------------------------
inline void PipelineVK::CreateSwapChain()
{
    // Update the swap chain extent to match the surface.
    if (m_surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        m_swapChainExtent = m_surfaceCapabilities.currentExtent;
    }

    // Determine the swap chain image count.
    uint32_t imageCount = m_surfaceCapabilities.minImageCount + 1;
    if (m_surfaceCapabilities.maxImageCount &&
        m_surfaceCapabilities.maxImageCount <= imageCount)
    {
        imageCount = m_surfaceCapabilities.maxImageCount;
    }

    // Describe the swap chain.
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.clipped = VK_TRUE;
    createInfo.surface = m_surface;
    createInfo.imageArrayLayers = 1;
    createInfo.minImageCount = imageCount;
    createInfo.presentMode = m_presentMode;
    createInfo.imageExtent = m_swapChainExtent;
    createInfo.imageFormat = m_surfaceFormat.format;
    createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.preTransform = m_surfaceCapabilities.currentTransform;
    const uint32_t queueFamilyIndices[] = { m_graphicsQueueFamilyIndex,
                                            m_presentQueueFamilyIndex };
    if (m_graphicsQueueFamilyIndex != m_presentQueueFamilyIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    // Create the swap chain.
    VULKAN_ENSURE(vkCreateSwapchainKHR(m_device,
                                       &createInfo,
                                       nullptr,
                                       &m_swapChain));

    // Get the count of swap chain images.
    VULKAN_ENSURE(vkGetSwapchainImagesKHR(m_device,
                                          m_swapChain,
                                          &imageCount,
                                          nullptr));

    // Get the swap chain images.
    m_swapChainImages.resize(imageCount);
    VULKAN_ENSURE(vkGetSwapchainImagesKHR(m_device,
                                          m_swapChain,
                                          &imageCount,
                                          m_swapChainImages.data()));
}

//--------------------------------------------------------------
inline VkImageView CreateImageView(const VkImage& a_image,
                                   const VkFormat& a_format,
                                   const VkDevice& a_device)
{
    // Describe the image view.
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.image = a_image;
    viewInfo.format = a_format;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    // Create the image view.
    VkImageView imageView;
    VULKAN_ENSURE(vkCreateImageView(a_device,
                                    &viewInfo,
                                    nullptr,
                                    &imageView));
    return imageView;
}

//--------------------------------------------------------------
inline void PipelineVK::CreateImageViews()
{
    const size_t numSwapChainImages = m_swapChainImages.size();
    m_swapChainImageViews.resize(numSwapChainImages);
    for (size_t i = 0; i < numSwapChainImages; ++i)
    {
        m_swapChainImageViews[i] = CreateImageView(m_swapChainImages[i],
                                                   m_surfaceFormat.format,
                                                   m_device);
    }
}

//--------------------------------------------------------------
inline void PipelineVK::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Describe the render pass.
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    // Create the render pass.
    VULKAN_ENSURE(vkCreateRenderPass(m_device,
                                     &renderPassInfo,
                                     nullptr,
                                     &m_renderPass));
}

//--------------------------------------------------------------
inline void PipelineVK::CreateDescriptorSetLayout()
{
    // Describe the ubo descriptor set layout binding.
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Describe the sampler descriptor set layout binding.
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Describe the descriptor set layout.
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding,
                                                             samplerLayoutBinding };
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    // Create the descriptor set layout.
    VULKAN_ENSURE(vkCreateDescriptorSetLayout(m_device,
                                              &layoutInfo,
                                              nullptr,
                                              &m_descriptorSetLayout));
}

//--------------------------------------------------------------
inline void CompileShader(const std::string& a_shaderSource,
                          const std::string& a_shaderName,
                          shaderc_shader_kind a_shaderKind,
                          std::vector<uint32_t>& o_buffer)
{
    // This shaderc::Compiler object results in a memory leak of
    // a single std::mutex (80 bytes on Windows x64) that is out
    // of our control until/unless it is fixed in shaderc itself.
    // https://github.com/google/shaderc/issues/1052#issuecomment-626848661
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(a_shaderSource,
                                                                     a_shaderKind,
                                                                     a_shaderName.c_str(),
                                                                     options);
    assert(result.GetCompilationStatus() == shaderc_compilation_status_success);
    o_buffer.assign(result.cbegin(), result.cend());
}

//--------------------------------------------------------------
inline const std::vector<uint32_t>& GetVertShaderBuffer()
{
    // The vertex shader source only needs to be compiled once.
    static std::vector<uint32_t> s_vertShaderBuffer;
    if (s_vertShaderBuffer.empty())
    {
        const std::string vertShaderSource =
        R"(
            #version 450

            layout(location = 0) in vec2 vertexPos;
            layout(location = 1) in vec2 vertexUV;
            layout(location = 0) out vec2 fragUV;

            void main()
            {
                gl_Position = vec4(vertexPos, 0.0, 1.0);
                fragUV = vertexUV;
            }
        )";
        CompileShader(vertShaderSource,
                      "vert_shader",
                      shaderc_glsl_vertex_shader,
                      s_vertShaderBuffer);
    }

    return s_vertShaderBuffer;
}

//--------------------------------------------------------------
inline const std::vector<uint32_t>& GetFragShaderBuffer()
{
    // The fragment shader source only needs to be compiled once.
    static std::vector<uint32_t> s_fragShaderBuffer;
    if (s_fragShaderBuffer.empty())
    {
        const std::string fragShaderSourceUint =
        R"(
            #version 450

            layout(location = 0) in vec2 fragUV;
            layout(location = 0) out vec4 color;
            layout(binding = 1) uniform sampler2D texSampler;

            void main()
            {
                color = texture(texSampler, fragUV);
            }
        )";
        CompileShader(fragShaderSourceUint,
                        "frag_shader",
                        shaderc_glsl_fragment_shader,
                      s_fragShaderBuffer);
    }

    return s_fragShaderBuffer;
}

//--------------------------------------------------------------
inline VkShaderModule CreateShaderModule(const VkDevice& a_device,
                                         const std::vector<uint32_t>& a_shaderCode)
{
    // Describe the shader module.
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.pCode = a_shaderCode.data();
    createInfo.codeSize = a_shaderCode.size() * sizeof(uint32_t);
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // Create the shader module.
    VkShaderModule shaderModule;
    VULKAN_ENSURE(vkCreateShaderModule(a_device,
                                       &createInfo,
                                       nullptr,
                                       &shaderModule));
    return shaderModule;
}

//--------------------------------------------------------------
inline void PipelineVK::CreateGraphicsPipeline()
{
    // Create the vertex shader module.
    const std::vector<uint32_t>& vertShaderBuffer = GetVertShaderBuffer();
    VkShaderModule vertShaderModule = CreateShaderModule(m_device, vertShaderBuffer);

    // Create the fragment shader module.
    const std::vector<uint32_t>& fragShaderBuffer = GetFragShaderBuffer();
    VkShaderModule fragShaderModule = CreateShaderModule(m_device, fragShaderBuffer);

    // Describe the vertex shader stage.
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    // Describe the fragment shader stage.
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    // Describe the vertex input binding.
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Describe the vertex input attributes.
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
    {
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, uv);
    }

    // Describe the vertex input state.
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // Describe the input assembly state.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    // Describe the viewport state.
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    // Describe the rasterization state.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    // Describe the multisample state.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    // Describe the color blend attachment.
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;

    // Describe the color blend state.
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    // Describe the dynamic state.
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    std::array<VkDynamicState, 2> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,
                                                    VK_DYNAMIC_STATE_SCISSOR };
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    // Describe the pipeline layout.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    // Create the pipeline layout.
    VULKAN_ENSURE(vkCreatePipelineLayout(m_device,
                                         &pipelineLayoutInfo,
                                         nullptr,
                                         &m_pipelineLayout));

    // Describe the graphics pipeline.
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // Create the graphics pipeline.
    VULKAN_ENSURE(vkCreateGraphicsPipelines(m_device,
                                            VK_NULL_HANDLE,
                                            1,
                                            &pipelineInfo,
                                            nullptr,
                                            &m_graphicsPipeline));

    // Destroy the vertex and fragment shader modules.
    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

//--------------------------------------------------------------
inline void PipelineVK::CreateFrameBuffers()
{
    const size_t numImageViews = m_swapChainImageViews.size();
    m_swapChainFrameBuffers.resize(numImageViews);

    for (size_t i = 0; i < numImageViews; ++i)
    {
        // Describe the frame buffer.
        VkFramebufferCreateInfo framebufferInfo{};
        VkImageView attachments[] = { m_swapChainImageViews[i] };
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        // Create the frame buffer.
        VULKAN_ENSURE(vkCreateFramebuffer(m_device,
                                          &framebufferInfo,
                                          nullptr,
                                          &m_swapChainFrameBuffers[i]));
    }
}

//--------------------------------------------------------------
inline void PipelineVK::CreateCommandPool()
{
    // Describe the command pool.
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // Create the command pool.
    VULKAN_ENSURE(vkCreateCommandPool(m_device,
                                      &poolInfo,
                                      nullptr,
                                      &m_commandPool));
}

//--------------------------------------------------------------
inline uint32_t FindMemoryType(const VkMemoryPropertyFlags& a_properties,
                               const VkPhysicalDevice& a_physicalDevice,
                               const uint32_t& a_typeFilter)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(a_physicalDevice,
                                        &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((a_typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & a_properties) == a_properties)
        {
            return i;
        }
    }

    assert(false);
    return 0;
}

//--------------------------------------------------------------
inline void PipelineVK::CreateTextureImage()
{
    // Describe the image.
    VkImageCreateInfo imageInfo = {};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_bufferConfig.width;
    imageInfo.extent.height = m_bufferConfig.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_bufferFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    // Create the image.
    VULKAN_ENSURE(vkCreateImage(m_device,
                                &imageInfo,
                                nullptr,
                                &m_textureImage));

    // Get the memory requirements.
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device,
                                 m_textureImage,
                                 &memRequirements);

    // Describe the memory.
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               m_physicalDevice,
                                               memRequirements.memoryTypeBits);
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    // Allocate the image memory.
    VULKAN_ENSURE(vkAllocateMemory(m_device,
                                   &allocInfo,
                                   nullptr,
                                   &m_textureImageMemory));

    // Bind the image memory.
    VULKAN_ENSURE(vkBindImageMemory(m_device,
                                    m_textureImage,
                                    m_textureImageMemory,
                                    0));
}

//--------------------------------------------------------------
inline void PipelineVK::CreateTextureImageView()
{
    m_textureImageView = CreateImageView(m_textureImage,
                                         m_bufferFormat,
                                         m_device);
}

//--------------------------------------------------------------
inline void PipelineVK::CreateTextureSampler()
{
    // Describe the sampler.
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // Create the sampler.
    VULKAN_ENSURE(vkCreateSampler(m_device,
                                  &samplerInfo,
                                  nullptr,
                                  &m_textureSampler));
}

//--------------------------------------------------------------
inline void PipelineVK::CreateSharedBuffer()
{
    if (m_bufferConfig.interop == Buffer::Interop::HOST)
    {
        m_interopVK = std::make_unique<InteropVKHost>(*this);
    }
    else if (m_bufferConfig.interop == Buffer::Interop::CUDA)
    {
    #ifdef CUDA_SUPPORTED
        m_interopVK = std::make_unique<InteropVKCuda>(*this);
    #endif // CUDA_SUPPORTED
    }
}

//--------------------------------------------------------------
inline void PipelineVK::CreateVertexBuffer()
{
    // Create the staging buffer.
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize vertexBufferSize = sizeof(QuadVertices[0]) *
                                           QuadVertices.size();
    CreateBuffer(stagingBuffer,
                 stagingBufferMemory,
                 vertexBufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copy vertices from the CPU to the GPU staging buffer.
    void* data;
    VULKAN_ENSURE(vkMapMemory(m_device,
                              stagingBufferMemory,
                              0,
                              vertexBufferSize,
                              0,
                              &data));
    memcpy(data, QuadVertices.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    // Create the vertex buffer.
    CreateBuffer(m_vertexBuffer,
                 m_vertexBufferMemory,
                 vertexBufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Copy vertices from the staging buffer to the final buffer.
    CopyBuffer(stagingBuffer,
               m_vertexBuffer,
               vertexBufferSize);

    // Destroy the staging buffer.
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

//--------------------------------------------------------------
inline void PipelineVK::CreateIndexBuffer()
{
    // Create the staging buffer.
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize indexBufferSize = sizeof(QuadIndices[0]) *
                                          QuadIndices.size();
    CreateBuffer(stagingBuffer,
                 stagingBufferMemory,
                 indexBufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copy indices from the CPU to the GPU staging buffer.
    void* data;
    VULKAN_ENSURE(vkMapMemory(m_device,
                              stagingBufferMemory,
                              0,
                              indexBufferSize,
                              0,
                              &data));
    memcpy(data, QuadIndices.data(), (size_t)indexBufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    // Create the index buffer.
    CreateBuffer(m_indexBuffer,
                 m_indexBufferMemory,
                 indexBufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Copy indices from the staging buffer to the final buffer.
    CopyBuffer(stagingBuffer,
               m_indexBuffer,
               indexBufferSize);

    // Destroy the staging buffer.
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

//--------------------------------------------------------------
inline void PipelineVK::CreateDescriptorPool()
{
    // Describe the descriptor pool sizes.
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = N;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = N;

    // Describe the descriptor pool.
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = N;

    // Create the descriptor pool.
    VULKAN_ENSURE(vkCreateDescriptorPool(m_device,
                                         &poolInfo,
                                         nullptr,
                                         &m_descriptorPool));
}

//--------------------------------------------------------------
inline void PipelineVK::CreateDescriptorSets()
{
    // Describe the descriptor set.
    VkDescriptorSetAllocateInfo allocInfo = {};
    std::vector<VkDescriptorSetLayout> layouts(N, m_descriptorSetLayout);
    allocInfo.descriptorSetCount = N;
    allocInfo.pSetLayouts = layouts.data();
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    // Create the descriptor sets.
    m_descriptorSets.resize(N);
    VULKAN_ENSURE(vkAllocateDescriptorSets(m_device,
                                           &allocInfo,
                                           m_descriptorSets.data()));

    // Update the descriptor sets.
    for (size_t n = 0; n < N; ++n)
    {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_textureImageView;
        imageInfo.sampler = m_textureSampler;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.dstSet = m_descriptorSets[n];
        descriptorWrite.dstBinding = 1;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
    }
}

//--------------------------------------------------------------
inline void PipelineVK::CreateCommandBuffers()
{
    // Describe the command buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = N;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    // Create the command buffers.
    m_commandBuffers.resize(N);
    VULKAN_ENSURE(vkAllocateCommandBuffers(m_device,
                                           &allocInfo,
                                           m_commandBuffers.data()));
}

//--------------------------------------------------------------
inline void PipelineVK::CreateSyncObjects()
{
    // Describe the semaphores.
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Describe the fences.
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // Create the semaphores and fences.
    m_imageAvailableSemaphores.resize(N);
    m_renderFinishedSemaphores.resize(N);
    m_inFlightFences.resize(N);
    for (size_t n = 0; n < N; ++n)
    {
        VULKAN_ENSURE(vkCreateSemaphore(m_device,
                                        &semaphoreInfo,
                                        nullptr,
                                        &m_imageAvailableSemaphores[n]));
        VULKAN_ENSURE(vkCreateSemaphore(m_device,
                                        &semaphoreInfo,
                                        nullptr,
                                        &m_renderFinishedSemaphores[n]));
        VULKAN_ENSURE(vkCreateFence(m_device,
                                    &fenceInfo,
                                    nullptr,
                                    &m_inFlightFences[n]));
    }
}

//--------------------------------------------------------------
inline VkDeviceSize PipelineVK::CreateBuffer(VkBuffer& a_buffer,
                                             VkDeviceMemory& a_bufferMemory,
                                             const VkDeviceSize a_bufferSize,
                                             const VkBufferUsageFlags a_bufferUsageFlags,
                                             const VkMemoryPropertyFlags a_memoryPropertyFlags,
                                             const void* a_bufferCreateInfoNext,
                                             const void* a_memoryAllocateInfoNext)
{
    // Describe the buffer.
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.size = a_bufferSize;
    bufferInfo.usage = a_bufferUsageFlags;
    bufferInfo.pNext = a_bufferCreateInfoNext;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    // Create the buffer.
    VULKAN_ENSURE(vkCreateBuffer(m_device,
                                 &bufferInfo,
                                 nullptr,
                                 &a_buffer));

    // Get the memory requirements.
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device,
                                  a_buffer,
                                  &memRequirements);

    // Describe the memory.
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.pNext = a_memoryAllocateInfoNext;
    allocInfo.memoryTypeIndex = FindMemoryType(a_memoryPropertyFlags,
                                               m_physicalDevice,
                                               memRequirements.memoryTypeBits);
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    // Allocate the buffer memory.
    VULKAN_ENSURE(vkAllocateMemory(m_device,
                                   &allocInfo,
                                   nullptr,
                                   &a_bufferMemory));

    // Bind the buffer memory.
    VULKAN_ENSURE(vkBindBufferMemory(m_device,
                                     a_buffer,
                                     a_bufferMemory,
                                     0));

    return allocInfo.allocationSize;
}



//--------------------------------------------------------------
inline void PipelineVK::CopyBuffer(const VkBuffer& a_sourceBuffer,
                                   const VkBuffer& a_destinationBuffer,
                                   const VkDeviceSize a_sourceBufferSize)
{
    // Describe the command buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    // Create the command buffer.
    VkCommandBuffer commandBuffer;
    VULKAN_ENSURE(vkAllocateCommandBuffers(m_device,
                                           &allocInfo,
                                           &commandBuffer));

    // Begin recording commands.
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VULKAN_ENSURE(vkBeginCommandBuffer(commandBuffer,
                                       &beginInfo));

    // Copy the buffer.
    VkBufferCopy copyRegion = {};
    copyRegion.size = a_sourceBufferSize;
    vkCmdCopyBuffer(commandBuffer,
                    a_sourceBuffer,
                    a_destinationBuffer,
                    1,
                    &copyRegion);

    // End recording commands.
    VULKAN_ENSURE(vkEndCommandBuffer(commandBuffer));

    // Describe the submit info.
    VkSubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Submit commands to the queue.
    VULKAN_ENSURE(vkQueueSubmit(m_graphicsQueue,
                                1,
                                &submitInfo,
                                VK_NULL_HANDLE));

    // Wait for the queue to process the commands.
    VULKAN_ENSURE(vkQueueWaitIdle(m_graphicsQueue));

    // Free the command buffer.
    vkFreeCommandBuffers(m_device,
                         m_commandPool,
                         1,
                         &commandBuffer);
}

//--------------------------------------------------------------
inline void TransitionImageLayout(const VkImage& a_image,
                                  const VkImageLayout& a_oldLayout,
                                  const VkImageLayout& a_newLayout,
                                  const VkCommandBuffer& a_commandBuffer)
{
    // Describe the memory barrier.
    VkImageMemoryBarrier barrier = {};
    barrier.image = a_image;
    barrier.oldLayout = a_oldLayout;
    barrier.newLayout = a_newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    // Set the pipeline state flags and barrier access masks.
    VkPipelineStageFlags sourceStage = 0;
    VkPipelineStageFlags destinationStage = 0;
    if (a_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        a_newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (a_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             a_newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    assert(destinationStage);
    assert(sourceStage);

    // Insert the memory barrier.
    vkCmdPipelineBarrier(a_commandBuffer,
                         sourceStage,
                         destinationStage,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);
}

//--------------------------------------------------------------
inline void PipelineVK::RenderFrame()
{
    // Wait for the last frame which used this index to complete.
    VULKAN_ENSURE(vkWaitForFences(m_device,
                                  1,
                                  &m_inFlightFences[m_currentFrameIndex],
                                  VK_TRUE,
                                  UINT64_MAX));

    // Get the next image index.
    uint32_t imageIndex;
    VULKAN_ENSURE(vkAcquireNextImageKHR(m_device,
                                        m_swapChain,
                                        UINT64_MAX,
                                        m_imageAvailableSemaphores[m_currentFrameIndex],
                                        VK_NULL_HANDLE,
                                        &imageIndex));

    // Reset the fences for this frame.
    VULKAN_ENSURE(vkResetFences(m_device,
                                1,
                                &m_inFlightFences[m_currentFrameIndex]));

    // Record all commands for this frame.
    {
        // Reset the command buffer for this frame.
        VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrameIndex];
        VULKAN_ENSURE(vkResetCommandBuffer(commandBuffer, 0));

        // Begin recording commands for this frame.
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VULKAN_ENSURE(vkBeginCommandBuffer(commandBuffer,
                                           &beginInfo));

        // Transition the texture image to a copy destination.
        TransitionImageLayout(m_textureImage,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              commandBuffer);

        // Describe the buffer copy.
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { m_bufferConfig.width,
                               m_bufferConfig.height,
                               1 };

        // Copy the shared buffer to the texture image.
        vkCmdCopyBufferToImage(commandBuffer,
                               m_sharedBuffer,
                               m_textureImage,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);

        // Transition the texture image back to read only.
        TransitionImageLayout(m_textureImage,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              commandBuffer);

        // Describe the render pass.
        VkRenderPassBeginInfo renderPassInfo = {};
        VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChainExtent;
        renderPassInfo.framebuffer = m_swapChainFrameBuffers[imageIndex];
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        // Begin the render pass.
        vkCmdBeginRenderPass(commandBuffer,
                             &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        // Bind the graphics pipeline to the command buffer.
        vkCmdBindPipeline(commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_graphicsPipeline);

        // Set the viewport.
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChainExtent.width);
        viewport.height = static_cast<float>(m_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // Set the scissor rect.
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Bind the vertex buffer.
        VkBuffer vertexBuffers[] = { m_vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer,
                               0,
                               1,
                               vertexBuffers,
                               offsets);

        // Bind the index buffer.
        vkCmdBindIndexBuffer(commandBuffer,
                             m_indexBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16);

        // Bind the descriptor sets.
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipelineLayout,
                                0,
                                1,
                                &m_descriptorSets[m_currentFrameIndex],
                                0,
                                nullptr);

        // Draw the indexed quad.
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(QuadIndices.size()),
                         1, 0, 0, 0);

        // End the render pass.
        vkCmdEndRenderPass(commandBuffer);

        // End recording commands for this frame.
        VULKAN_ENSURE(vkEndCommandBuffer(commandBuffer));
    }

    // Describe the submit info.
    VkSubmitInfo submitInfo = {};
    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrameIndex] };
    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrameIndex] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrameIndex];
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Submit all commands to the queue.
    VULKAN_ENSURE(vkQueueSubmit(m_graphicsQueue,
                                1, 
                                &submitInfo,
                                m_inFlightFences[m_currentFrameIndex]));

    // Describe the present.
    VkSwapchainKHR swapChains[] = { m_swapChain };
    VkPresentInfoKHR presentInfo = {};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Present the rendered image on the display.
    VULKAN_ENSURE(vkQueuePresentKHR(m_presentQueue,
                                    &presentInfo));

    // Cycle to the next frame index.
    m_currentFrameIndex = (m_currentFrameIndex + 1) % N;
}

} // namespace Vulkan
} // namespace Display
} // namespace Simple
