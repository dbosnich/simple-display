//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <display/context_implementation.h>

#ifdef OPENGL_SUPPORTED
#include "context_linux_gl.h"
#endif

#ifdef VULKAN_SUPPORTED
#include "context_linux_vk.h"
#endif

using namespace Simple::Display;
using ImplPtr = std::unique_ptr<Context::Implementation>;

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
#ifdef OPENGL_SUPPORTED
    return CreateOpenGL(a_config);
#elif VULKAN_SUPPORTED
    return CreateVulkan(a_config);
#else
    printf("Cannot create Linux Context Implementation.\n"
           "Please ensure the SimpleDisplay library was \n"
           "built using a valid OpenGL SDK or Vulkan SDK.\n\n");
    (void)a_config;
    return nullptr;
#endif
}

//--------------------------------------------------------------
inline ImplPtr CreateOpenGL(const Context::Config& a_config)
{
#ifdef OPENGL_SUPPORTED
    return std::make_unique<OpenGL::ContextLinuxGL>(a_config);
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
    return std::make_unique<Vulkan::ContextLinuxVK>(a_config);
#else
    printf("Cannot create Vulkan Context Implementation.\n"
           "Please ensure the SimpleDisplay library was \n"
           "built using a valid Vulkan SDK installation.\n\n");
    (void)a_config;
    return nullptr;
#endif
}
