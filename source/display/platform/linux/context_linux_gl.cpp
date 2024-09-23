//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#ifdef OPENGL_SUPPORTED

#include "context_linux_gl.h"

#define GLAD_GL_IMPLEMENTATION
#include <display/graphics/opengl/glad/gl_core_4.6.h>
#include <display/graphics/opengl/buffer_gl_core.h>
#include <GL/glx.h>

using namespace Simple::Display;
using namespace Simple::Display::OpenGL;

//--------------------------------------------------------------
ContextLinuxGL::ContextLinuxGL(const Context::Config& a_config)
{
    // Create the window.
    m_window = new Window(a_config.windowConfig);

    // Get the native display handle.
    ::Display* nativeDisplay = (::Display*)m_window->GetNativeDisplayHandle();
    assert(nativeDisplay);

    // Get the native window handle.
    ::Window* nativeWindow = (::Window*)m_window->GetNativeWindowHandle();
    assert(nativeWindow);

    // Create the rendering context.
    GLint attributes[] = { GLX_RGBA,
                           GLX_DEPTH_SIZE,
                           24,
                           GLX_DOUBLEBUFFER,
                           None };
    XVisualInfo* visualInfo = glXChooseVisual(nativeDisplay, 0, attributes);
    m_glxContext = ::glXCreateContext(nativeDisplay,
                                      visualInfo,
                                      nullptr,
                                      True);
    assert(m_glxContext != nullptr);
    XFree(visualInfo);

    // Activate the rendering context.
    ::glXMakeCurrent(nativeDisplay,
                     *nativeWindow,
                     m_glxContext);
    assert(::glXGetCurrentContext() == m_glxContext);

    // Initialize glad.
    gladLoaderLoadGL();

    // Create the buffer.
    using namespace std;
    using BufferImpl = BufferGLCore;
    const Buffer::Config& bufferConfig = a_config.bufferConfig;
    m_buffer = new Buffer(make_unique<BufferImpl>(bufferConfig));

    // Show the window.
    m_window->Show();
}

//--------------------------------------------------------------
ContextLinuxGL::~ContextLinuxGL()
{
    // Hide the window.
    assert(m_window != nullptr);
    m_window->Hide();

    // Destroy the buffer.
    delete m_buffer;
    m_buffer = nullptr;

    // Get the native display handle.
    ::Display* nativeDisplay = (::Display*)m_window->GetNativeDisplayHandle();
    assert(nativeDisplay);

    // Deactivate the rendering context.
    ::glXMakeCurrent(nativeDisplay, None, nullptr);

    // Destroy the rendering context.
    assert(m_glxContext != nullptr);
    ::glXDestroyContext(nativeDisplay, m_glxContext);
    m_glxContext = nullptr;

    // Destroy the window.
    delete m_window;
    m_window = nullptr;
}

//--------------------------------------------------------------
Buffer& ContextLinuxGL::GetBuffer() const
{
    return *m_buffer;
}

//--------------------------------------------------------------
Simple::Display::Window* ContextLinuxGL::GetWindow() const
{
    return m_window;
}

//--------------------------------------------------------------
void ContextLinuxGL::OnFrameStart()
{
    // Process all pending window events.
    m_window->PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void ContextLinuxGL::OnFrameEnded()
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

    // Get the native display handle.
    ::Display* nativeDisplay = (::Display*)m_window->GetNativeDisplayHandle();
    assert(nativeDisplay);

    // Get the native window handle.
    ::Window* nativeWindow = (::Window*)m_window->GetNativeWindowHandle();
    assert(nativeWindow);

    // Present the rendered image on the display.
    glXSwapBuffers(nativeDisplay, *nativeWindow);
}

#endif // OPENGL_SUPPORTED
