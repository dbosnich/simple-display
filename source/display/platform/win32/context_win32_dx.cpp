//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include "context_win32_dx.h"

#include <display/graphics/d3d12/buffer_d3d12.h>

using namespace Simple::Display;
using namespace Simple::Display::DirectX;

//--------------------------------------------------------------
ContextWin32DX::ContextWin32DX(const Context::Config& a_config)
{
    // Create the window.
    m_window = new Window(a_config.windowConfig);

    // Get the native window handle.
    HWND windowHandle = (HWND)m_window->GetNativeWindowHandle();
    assert(windowHandle);

    // Create the buffer.
    using namespace std;
    using BufferImpl = BufferD3D12;
    const Buffer::Config& bufferConfig = a_config.bufferConfig;
    m_buffer = new Buffer(make_unique<BufferImpl>(bufferConfig,
                                                  windowHandle));

    // Show the window.
    m_window->Show();
}

//--------------------------------------------------------------
ContextWin32DX::~ContextWin32DX()
{
    // Hide the window.
    assert(m_window != nullptr);
    m_window->Hide();

    // Destroy the buffer.
    delete m_buffer;
    m_buffer = nullptr;

    // Destroy the window.
    delete m_window;
    m_window = nullptr;
}

//--------------------------------------------------------------
Buffer& ContextWin32DX::GetBuffer() const
{
    return *m_buffer;
}

//--------------------------------------------------------------
Window* ContextWin32DX::GetWindow() const
{
    return m_window;
}

//--------------------------------------------------------------
void ContextWin32DX::OnFrameStart()
{
    // Process all pending window events.
    m_window->PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void ContextWin32DX::OnFrameEnded()
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
