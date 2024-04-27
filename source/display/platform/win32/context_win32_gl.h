//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/context_implementation.h>

#define NOMINMAX
#include <windows.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class ContextWin32GL : public Context::Implementation
{
public:
    ContextWin32GL(const Context::Config& a_config);
    ~ContextWin32GL() override;

    ContextWin32GL(const ContextWin32GL&) = delete;
    ContextWin32GL& operator=(const ContextWin32GL&) = delete;

protected:
    Buffer& GetBuffer() const override;
    Window* GetWindow() const override;

    void OnFrameStart() override;
    void OnFrameEnded() override;

private:
    Buffer* m_buffer = nullptr;
    Window* m_window = nullptr;
    HDC m_deviceContext = nullptr;
    HGLRC m_openGLContext = nullptr;
};

} // namespace OpenGL
} // namespace Display
} // namespace Simple
