//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#ifdef VULKAN_SUPPORTED

#include "context_win32_vk.h"

#include <display/graphics/vulkan/buffer_vk.h>
#include <display/graphics/vulkan/debug_vk.h>

using namespace Simple::Display;
using namespace Simple::Display::Vulkan;

//--------------------------------------------------------------
void CreatePipelineContext(PipelineContext*& a_pipelineContext,
                           const Buffer::Config& a_bufferConfig,
                           const Window& a_window);
void DestroyPipelineContext(PipelineContext*& a_pipelineContext);

//--------------------------------------------------------------
ContextWin32VK::ContextWin32VK(const Context::Config& a_config)
{
    // Create the window.
    m_window = new Window(a_config.windowConfig);

    // Create the pipeline context.
    const Buffer::Config& bufferConfig = a_config.bufferConfig;
    CreatePipelineContext(m_pipelineContext,
                          bufferConfig,
                          *m_window);
    assert(m_pipelineContext);

    // Create the buffer.
    using namespace std;
    using BufferImpl = BufferVK;
    m_buffer = new Buffer(make_unique<BufferImpl>(bufferConfig,
                                                  *m_pipelineContext));

    // Show the window.
    m_window->Show();
}

//--------------------------------------------------------------
ContextWin32VK::~ContextWin32VK()
{
    // Hide the window.
    assert(m_window != nullptr);
    m_window->Hide();

    // Destroy the buffer.
    delete(m_buffer);
    m_buffer = nullptr;

    // Destroy the pipeline context.
    DestroyPipelineContext(m_pipelineContext);
    assert(m_pipelineContext == nullptr);

    // Destroy the window.
    delete(m_window);
    m_window = nullptr;
}

//--------------------------------------------------------------
Buffer& ContextWin32VK::GetBuffer() const
{
    return *m_buffer;
}

//--------------------------------------------------------------
Window* ContextWin32VK::GetWindow() const
{
    return m_window;
}

//--------------------------------------------------------------
void ContextWin32VK::OnFrameStart()
{
    // Process all pending window events.
    m_window->PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void ContextWin32VK::OnFrameEnded()
{
    if (m_window->IsMinimized() || m_window->IsClosed())
    {
        return;
    }

    // Get the current window dimensions.
    uint32_t displayWidth = 0;
    uint32_t displayHeight = 0;
    m_window->GetDisplayDimensions(displayWidth, displayHeight);

    // Render the pixel buffer.
    m_buffer->Render(displayWidth, displayHeight);
}

#if VULKAN_DEBUG_SETTING
//--------------------------------------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT a_severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT a_type,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* a_callbackData,
                                                    void* a_userData)
{
    (void)a_userData;
    printf("Vulkan Debug Message\n"
           "  type:     0x%x\n"
           "  severity: 0x%x\n"
           "  message:  %s\n\n",
           a_type, a_severity, a_callbackData->pMessage);
    assert(a_severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT);
    return VK_FALSE;
}

//--------------------------------------------------------------
void CreateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT a_createInfo,
                          VkDebugUtilsMessengerEXT& o_debugMessenger,
                          VkInstance a_instance)
{
    using CreateFunction = PFN_vkCreateDebugUtilsMessengerEXT;
    auto createFunction = (CreateFunction)vkGetInstanceProcAddr(a_instance,
                                                                "vkCreateDebugUtilsMessengerEXT");
    const VkResult result = createFunction ?
                            createFunction(a_instance,
                                           &a_createInfo,
                                           nullptr,
                                           &o_debugMessenger) :
                            VK_ERROR_EXTENSION_NOT_PRESENT;
    if (result != VK_SUCCESS)
    {
        printf("Could not create Vulkan Debug Messenger!\n\n");
    }
}

//--------------------------------------------------------------
void DestroyDebugMessenger(VkDebugUtilsMessengerEXT a_debugMessenger,
                           VkInstance a_instance)
{
    using DestroyFunction = PFN_vkDestroyDebugUtilsMessengerEXT;
    auto destroyFunction = (DestroyFunction)vkGetInstanceProcAddr(a_instance,
                                                                  "vkDestroyDebugUtilsMessengerEXT");
    if (destroyFunction != nullptr)
    {
        destroyFunction(a_instance, a_debugMessenger, nullptr);
    }
}
#endif

//--------------------------------------------------------------
void CreatePipelineContext(PipelineContext*& a_pipelineContext,
                           const Buffer::Config& a_bufferConfig,
                           const Window& a_window)
{
    // Create the context.
    assert(a_pipelineContext == nullptr);
    a_pipelineContext = new PipelineContext();

    // List the extensions to enable.
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    if (a_bufferConfig.interop == Buffer::Interop::CUDA)
    {
        extensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        a_pipelineContext->requiredDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        a_pipelineContext->requiredDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
        a_pipelineContext->externalMemoryHandleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    }
#if VULKAN_DEBUG_SETTING
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    // Describe the instance.
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
#if VULKAN_DEBUG_SETTING
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    constexpr int verbose = VULKAN_DEBUG_SETTING == VULKAN_DEBUG_VERBOSE;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                      (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT & verbose);
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugMessageCallback;
    createInfo.pNext = &debugCreateInfo;
#endif

    // Create the instance.
    VULKAN_ENSURE(vkCreateInstance(&createInfo,
                                   nullptr,
                                   &a_pipelineContext->instance));
    assert(a_pipelineContext->instance);

    // Create the debug messenger.
#if VULKAN_DEBUG_SETTING
    CreateDebugMessenger(debugCreateInfo,
                         a_pipelineContext->debugMessenger,
                         a_pipelineContext->instance);
    assert(a_pipelineContext->debugMessenger);
#endif

    // Set the display extents.
    a_window.GetDisplayDimensions(a_pipelineContext->displayExtent.width,
                                  a_pipelineContext->displayExtent.height);

    // Get the native window handle.
    HWND windowHandle = (HWND)a_window.GetNativeWindowHandle();
    assert(windowHandle);

    // Describe the surface.
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hinstance = nullptr;
    surfaceCreateInfo.hwnd = windowHandle;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

    // Create the surface.
    VULKAN_ENSURE(vkCreateWin32SurfaceKHR(a_pipelineContext->instance,
                                          &surfaceCreateInfo,
                                          nullptr,
                                          &a_pipelineContext->surface));
}

//--------------------------------------------------------------
void DestroyPipelineContext(PipelineContext*& a_pipelineContext)
{
    assert(a_pipelineContext);
    assert(a_pipelineContext->surface);
    assert(a_pipelineContext->instance);

    // Destroy the surface.
    vkDestroySurfaceKHR(a_pipelineContext->instance,
                        a_pipelineContext->surface,
                        nullptr);
    a_pipelineContext->surface = nullptr;

    // Destroy the debug messenger.
#if VULKAN_DEBUG_SETTING
    assert(a_pipelineContext->debugMessenger);
    DestroyDebugMessenger(a_pipelineContext->debugMessenger,
                          a_pipelineContext->instance);
    a_pipelineContext->debugMessenger = nullptr;
#endif

    // Destroy the instance.
    vkDestroyInstance(a_pipelineContext->instance, nullptr);
    a_pipelineContext->instance = nullptr;

    // Destroy the context.
    delete a_pipelineContext;
    a_pipelineContext = nullptr;
}

#endif // VULKAN_SUPPORTED
