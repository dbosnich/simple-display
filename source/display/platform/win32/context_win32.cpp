//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include "context_win32_dx.h"

#ifdef OPENGL_SUPPORTED
#include "context_win32_gl.h"
#endif

#ifdef VULKAN_SUPPORTED
#include "context_win32_vk.h"
#endif

using namespace Simple::Display;
using ImplPtr = std::unique_ptr<Context::Implementation>;

// Ensure the application runs on a dedicated gpu if available.
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

//--------------------------------------------------------------
ImplPtr CreateNative(const Context::Config& a_config);
ImplPtr CreateOpenGL(const Context::Config& a_config);
ImplPtr CreateVulkan(const Context::Config& a_config);

//--------------------------------------------------------------
ImplPtr Context::Implementation::Create(const Config& a_config)
{
    switch (a_config.graphicsAPI)
    {
        case GraphicsAPI::NATIVE: return CreateNative(a_config);
        case GraphicsAPI::OPENGL: return CreateOpenGL(a_config);
        case GraphicsAPI::VULKAN: return CreateVulkan(a_config);
        default: return nullptr;
    }
}

//--------------------------------------------------------------
inline ImplPtr CreateNative(const Context::Config& a_config)
{
    return std::make_unique<DirectX::ContextWin32DX>(a_config);
}

//--------------------------------------------------------------
inline ImplPtr CreateOpenGL(const Context::Config& a_config)
{
#ifdef OPENGL_SUPPORTED
    return std::make_unique<OpenGL::ContextWin32GL>(a_config);
#else
    printf("Cannot create OpenGL Context Implementation.\n"
           "Please ensure the SimpleDisplay library was \n"
           "built using a valid OpenGL SDK installation.\n\n");
    (void)a_config;
    return nullptr;
#endif
}

//--------------------------------------------------------------
inline ImplPtr CreateVulkan(const Context::Config& a_config)
{
#ifdef VULKAN_SUPPORTED
    return std::make_unique<Vulkan::ContextWin32VK>(a_config);
#else
    printf("Cannot create Vulkan Context Implementation.\n"
           "Please ensure the SimpleDisplay library was \n"
           "built using a valid Vulkan SDK installation.\n\n");
    (void)a_config;
    return nullptr;
#endif
}
