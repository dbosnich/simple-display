//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include "context_macos_mt.h"

#include <display/graphics/metal/buffer_mt.h>

#import <MetalKit/MTKView.h>

using namespace Simple::Display;
using namespace Simple::Display::Metal;

//--------------------------------------------------------------
ContextMacOSMT::ContextMacOSMT(const Context::Config& a_config)
{
    // Create the window.
    m_window = new Window(a_config.windowConfig);

    @autoreleasepool
    {
        // Create the Metal view.
        m_metalView = [MTKView alloc];
        NSRect rect = NSMakeRect(0,
                                 0,
                                 a_config.windowConfig.initialWidth,
                                 a_config.windowConfig.initialHeight);
        [m_metalView initWithFrame: rect
                            device: MTLCreateSystemDefaultDevice()];

        // Get the native window handle.
        NSWindow* nsWindow = (NSWindow*)m_window->GetNativeWindowHandle();
        assert(nsWindow);

        // Set the Metal view.
        [nsWindow setContentView: m_metalView];
        [nsWindow makeFirstResponder: m_metalView];
    }

    // Create the buffer.
    using namespace std;
    using BufferImpl = BufferMT;
    const Buffer::Config& bufferConfig = a_config.bufferConfig;
    m_buffer = new Buffer(make_unique<BufferImpl>(bufferConfig,
                                                  m_metalView));

    // Show the window.
    m_window->Show();
}

//--------------------------------------------------------------
ContextMacOSMT::~ContextMacOSMT()
{
    // Hide the window.
    assert(m_window != nullptr);
    m_window->Hide();

    // Destroy the buffer.
    delete m_buffer;
    m_buffer = nullptr;

    // Release the Metal view.
    [m_metalView release];
    m_metalView = nullptr;

    // Destroy the window.
    delete m_window;
    m_window = nullptr;
}

//--------------------------------------------------------------
Buffer& ContextMacOSMT::GetBuffer() const
{
    return *m_buffer;
}

//--------------------------------------------------------------
Window* ContextMacOSMT::GetWindow() const
{
    return m_window;
}

//--------------------------------------------------------------
void ContextMacOSMT::OnFrameStart()
{
    // Process all pending window events.
    m_window->PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void ContextMacOSMT::OnFrameEnded()
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
