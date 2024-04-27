//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#ifdef OPENGL_SUPPORTED

#include "context_macos_gl.h"

// MacOS only supports the OpenGL Core Profile up to version 4.1
#define GLAD_GL_IMPLEMENTATION
#include <display/graphics/opengl/glad/gl_core_4.1.h>
#include <display/graphics/opengl/buffer_gl_core.h>
#include <OpenGL/CGLCurrent.h>
#import <Cocoa/Cocoa.h>

using namespace Simple::Display;
using namespace Simple::Display::OpenGL;

//--------------------------------------------------------------
ContextMacOSGL::ContextMacOSGL(const Context::Config& a_config)
{
    // Create the window.
    m_window = new Window(a_config.windowConfig);

    @autoreleasepool
    {
        // Describe the pixel format of the drawing surface.
        NSOpenGLPixelFormat* pixelFormat = [NSOpenGLPixelFormat alloc];
        NSOpenGLPixelFormatAttribute pfa[] = {NSOpenGLPFADoubleBuffer,
                                              NSOpenGLPFADepthSize,
                                              (NSOpenGLPixelFormatAttribute)32,
                                              NSOpenGLPFAOpenGLProfile,
                                              NSOpenGLProfileVersion4_1Core,
                                              0};
        [pixelFormat initWithAttributes : pfa];

        // Create the OpenGL view.
        m_nsOpenGLView = [NSOpenGLView alloc];
        NSRect rect = NSMakeRect(0,
                                 0,
                                 a_config.windowConfig.initialWidth,
                                 a_config.windowConfig.initialHeight);
        [m_nsOpenGLView initWithFrame: rect
                        pixelFormat: pixelFormat];

        // Get the native window handle.
        NSWindow* nsWindow = (NSWindow*)m_window->GetNativeWindowHandle();
        assert(nsWindow);

        // Set the OpenGL view.
        [nsWindow setContentView: m_nsOpenGLView];
        [nsWindow makeFirstResponder: m_nsOpenGLView];
        [[m_nsOpenGLView openGLContext] makeCurrentContext];
        [[m_nsOpenGLView openGLContext] setView: m_nsOpenGLView];
        assert(m_nsOpenGLView.openGLContext.CGLContextObj == CGLGetCurrentContext());
    }

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
ContextMacOSGL::~ContextMacOSGL()
{
    // Hide the window.
    assert(m_window != nullptr);
    m_window->Hide();

    // Destroy the buffer.
    delete(m_buffer);
    m_buffer = nullptr;

    // Release the OpenGL view.
    [m_nsOpenGLView release];
    m_nsOpenGLView = nullptr;

    // Destroy the window.
    delete(m_window);
    m_window = nullptr;
}

//--------------------------------------------------------------
Buffer& ContextMacOSGL::GetBuffer() const
{
    return *m_buffer;
}

//--------------------------------------------------------------
Window* ContextMacOSGL::GetWindow() const
{
    return m_window;
}

//--------------------------------------------------------------
void ContextMacOSGL::OnFrameStart()
{
    // Process all pending window events.
    m_window->PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void ContextMacOSGL::OnFrameEnded()
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
    [[m_nsOpenGLView openGLContext] flushBuffer];
}

#endif // OPENGL_SUPPORTED
