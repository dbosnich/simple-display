//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#ifdef VULKAN_SUPPORTED

#include "context_macos_vk.h"

#include <display/graphics/vulkan/buffer_vk.h>

#import <MetalKit/MTKView.h>

//--------------------------------------------------------------
//! Controls whether any Vulkan debug messages should be output.
//--------------------------------------------------------------
#define VULKAN_DEBUG_NOTHING 0
#define VULKAN_DEBUG_DEFAULT 1
#define VULKAN_DEBUG_VERBOSE 2
#ifndef VULKAN_DEBUG_SETTING
#   ifdef NDEBUG
#       define VULKAN_DEBUG_SETTING VULKAN_DEBUG_NOTHING
#   else
#       define VULKAN_DEBUG_SETTING VULKAN_DEBUG_DEFAULT
#   endif
#endif//VULKAN_DEBUG_SETTING

using namespace Simple::Display;
using namespace Simple::Display::Vulkan;

//--------------------------------------------------------------
void CreatePipelineContext(PipelineContext*& a_pipelineContext,
                           const MTKView* a_mtkView,
                           const Window& a_window);
void DestroyPipelineContext(PipelineContext*& a_pipelineContext);

//--------------------------------------------------------------
ContextMacOSVK::ContextMacOSVK(const Context::Config& a_config)
{
    // Create the window.
    m_window = new Window(a_config.windowConfig);

    @autoreleasepool
    {
        // Create the Metal view.
        m_mtkView = [MTKView alloc];
        NSRect rect = NSMakeRect(0,
                                 0,
                                 a_config.windowConfig.initialWidth,
                                 a_config.windowConfig.initialHeight);
        [m_mtkView initWithFrame: rect
                   device: MTLCreateSystemDefaultDevice()];

        // Get the native window handle.
        NSWindow* nsWindow = (NSWindow*)m_window->GetNativeWindowHandle();
        assert(nsWindow);

        // Set the Metal view.
        [nsWindow setContentView: m_mtkView];
        [nsWindow makeFirstResponder: m_mtkView];
    }

    // Create the pipeline context.
    CreatePipelineContext(m_pipelineContext,
                          m_mtkView,
                          *m_window);
    assert(m_pipelineContext);

    // Create the buffer.
    using namespace std;
    using BufferImpl = BufferVK;
    const Buffer::Config& bufferConfig = a_config.bufferConfig;
    m_buffer = new Buffer(make_unique<BufferImpl>(bufferConfig,
                                                  *m_pipelineContext));

    // Show the window.
    m_window->Show();
}

//--------------------------------------------------------------
ContextMacOSVK::~ContextMacOSVK()
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

    // Release the Metal view.
    [m_mtkView release];
    m_mtkView = nullptr;

    // Destroy the window.
    delete(m_window);
    m_window = nullptr;
}

//--------------------------------------------------------------
Buffer& ContextMacOSVK::GetBuffer() const
{
    return *m_buffer;
}

//--------------------------------------------------------------
Window* ContextMacOSVK::GetWindow() const
{
    return m_window;
}

//--------------------------------------------------------------
void ContextMacOSVK::OnFrameStart()
{
    // Process all pending window events.
    m_window->PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void ContextMacOSVK::OnFrameEnded()
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
    assert(a_severity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT);
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
                           const MTKView* a_mtkView,
                           const Window& a_window)
{
    // Create the context.
    assert(a_pipelineContext == nullptr);
    a_pipelineContext = new PipelineContext();
    a_pipelineContext->requiredDeviceExtensions.push_back("VK_KHR_portability_subset");

    // List the extensions to enable.
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#if VULKAN_DEBUG_SETTING
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    // Describe the instance.
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
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
    AssertSucceeded(vkCreateInstance(&createInfo,
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

    // Describe the surface.
    VkMetalSurfaceCreateInfoEXT surfaceCreateInfo;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.pLayer = static_cast<CAMetalLayer*>(a_mtkView.layer);
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;

    // Create the surface.
    AssertSucceeded(vkCreateMetalSurfaceEXT(a_pipelineContext->instance,
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
