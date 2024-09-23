//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#ifdef OPENGL_SUPPORTED

#include "context_win32_gl.h"

//--------------------------------------------------------------
//! Whether to compile with the legacy GL compatibility profile.
//! This is not supported on all platforms and the default core
//! profile buffer implementation is more efficient regardless.
//! The option to enable the compatbility buffer implementation
//! instead only remains because it is a simpler implementation
//! that may be useful for the purposes of comparison/reference.
//--------------------------------------------------------------
#ifndef OPENGL_USE_LEGACY_COMPATIBILITY_PROFILE_IMPLEMENTATION
#define OPENGL_USE_LEGACY_COMPATIBILITY_PROFILE_IMPLEMENTATION 0
#endif//OPENGL_USE_LEGACY_COMPATIBILITY_PROFILE_IMPLEMENTATION

// Include the appropriate OpenGL headers, depending on whether
// the compatibility profile was requested in place of the core.
#pragma warning(push)
#pragma warning(disable:4551)
#define GLAD_GL_IMPLEMENTATION
#if OPENGL_USE_LEGACY_COMPATIBILITY_PROFILE_IMPLEMENTATION
#   include <display/graphics/opengl/glad/gl_compat_4.6.h>
#   include <display/graphics/opengl/buffer_gl_compat.h>
#else
#   include <display/graphics/opengl/glad/gl_core_4.6.h>
#   include <display/graphics/opengl/buffer_gl_core.h>
#endif // OPENGL_USE_LEGACY_COMPATIBILITY_PROFILE_IMPLEMENTATION
#pragma warning(pop)

#include <display/graphics/opengl/debug_gl.h>

using namespace Simple::Display;
using namespace Simple::Display::OpenGL;

//--------------------------------------------------------------
void GLAPIENTRY DebugMessageCallback(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar* message,
                                     const void* userParam)
{
    (void)id;
    (void)length;
    (void)userParam;
    if (type != GL_DEBUG_TYPE_PERFORMANCE &&
        severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        printf("GL Debug Message\n"
               "  type:     0x%x\n"
               "  source:   0x%x\n"
               "  severity: 0x%x\n"
               "  message:  %s\n\n",
               type, source, severity, message);
        assert(false);
    }
}

//--------------------------------------------------------------
ContextWin32GL::ContextWin32GL(const Context::Config& a_config)
{
    // Create the window.
    m_window = new Window(a_config.windowConfig);

    // Describe the pixel format of the drawing surface.
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    // Get the native window handle.
    HWND hwnd = (HWND)m_window->GetNativeWindowHandle();
    assert(hwnd);

    // Get the device context.
    m_deviceContext = ::GetDC(hwnd);
    assert(m_deviceContext != nullptr);

    // Find a matching pixel format and set it.
    const int fmt = ::ChoosePixelFormat(m_deviceContext, &pfd);
    assert(fmt != 0);
    ::SetPixelFormat(m_deviceContext, fmt, &pfd);

    // Create the rendering context.
    m_openGLContext = ::wglCreateContext(m_deviceContext);
    assert(m_openGLContext != nullptr);

    // Activate the rendering context.
    ::wglMakeCurrent(m_deviceContext, m_openGLContext);
    assert(::wglGetCurrentContext() == m_openGLContext);

    // Initialize glad.
    gladLoaderLoadGL();

    // Enable debug info.
#if defined(GL_DEBUG_OUTPUT) && OPENGL_DEBUG_SETTING
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugMessageCallback, 0);
#endif

    // Select which buffer implementation to use.
#if OPENGL_USE_LEGACY_COMPATIBILITY_PROFILE_IMPLEMENTATION
    using BufferImpl = BufferGLCompat;
#else
    using BufferImpl = BufferGLCore;
#endif // OPENGL_USE_LEGACY_COMPATIBILITY_PROFILE_IMPLEMENTATION

    // Create the buffer.
    using namespace std;
    const Buffer::Config& bufferConfig = a_config.bufferConfig;
    m_buffer = new Buffer(make_unique<BufferImpl>(bufferConfig));

    // Show the window.
    m_window->Show();
}

//--------------------------------------------------------------
ContextWin32GL::~ContextWin32GL()
{
    // Hide the window.
    assert(m_window != nullptr);
    m_window->Hide();

    // Destroy the buffer.
    delete m_buffer;
    m_buffer = nullptr;

    // Deactivate the rendering context.
    ::wglMakeCurrent(nullptr, nullptr);

    // Delete the rendering context.
    assert(m_openGLContext != nullptr);
    ::wglDeleteContext(m_openGLContext);
    m_openGLContext = nullptr;

    // Get the native window handle.
    HWND hwnd = (HWND)m_window->GetNativeWindowHandle();
    assert(hwnd);

    // Release the device context.
    assert(m_deviceContext != nullptr);
    ::ReleaseDC(hwnd, m_deviceContext);
    m_deviceContext = nullptr;

    // Destroy the window.
    delete m_window;
    m_window = nullptr;
}

//--------------------------------------------------------------
Buffer& ContextWin32GL::GetBuffer() const
{
    return *m_buffer;
}

//--------------------------------------------------------------
Window* ContextWin32GL::GetWindow() const
{
    return m_window;
}

//--------------------------------------------------------------
void ContextWin32GL::OnFrameStart()
{
    // Process all pending window events.
    m_window->PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void ContextWin32GL::OnFrameEnded()
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

    // Present the rendered image on the display.
    ::SwapBuffers(m_deviceContext);
}

#endif // OPENGL_SUPPORTED
