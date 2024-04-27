//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include "context_macos_mt.h"

#ifdef OPENGL_SUPPORTED
#include "context_macos_gl.h"
#endif

#ifdef VULKAN_SUPPORTED
#include "context_macos_vk.h"
#endif

#import <Foundation/NSThread.h>

using namespace Simple::Display;
using ImplPtr = std::unique_ptr<Context::Implementation>;

//--------------------------------------------------------------
ImplPtr CreateNative(const Context::Config& a_config);
ImplPtr CreateOpenGL(const Context::Config& a_config);
ImplPtr CreateVulkan(const Context::Config& a_config);

//--------------------------------------------------------------
ImplPtr Context::Implementation::Create(const Config& a_config)
{
    if (!NSThread.isMainThread)
    {
        printf("MacOS Context Implementations can only \n"
               "be created from the main (UI) thread.\n\n");
        return nullptr;
    }

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
    return std::make_unique<Metal::ContextMacOSMT>(a_config);
}

//--------------------------------------------------------------
inline ImplPtr CreateOpenGL(const Context::Config& a_config)
{
#ifdef OPENGL_SUPPORTED
    return std::make_unique<OpenGL::ContextMacOSGL>(a_config);
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
    return std::make_unique<Vulkan::ContextMacOSVK>(a_config);
#else
    printf("Cannot create Vulkan Context Implementation.\n"
           "Please ensure the SimpleDisplay library was \n"
           "built using a valid Vulkan SDK installation.\n\n");
    (void)a_config;
    return nullptr;
#endif
}
