//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/context_implementation.h>

@class NSOpenGLView;

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class ContextMacOSGL : public Context::Implementation
{
public:
    ContextMacOSGL(const Context::Config& a_config);
    ~ContextMacOSGL() override;

    ContextMacOSGL(const ContextMacOSGL&) = delete;
    ContextMacOSGL& operator=(const ContextMacOSGL&) = delete;

protected:
    Buffer& GetBuffer() const override;
    Window* GetWindow() const override;

    void OnFrameStart() override;
    void OnFrameEnded() override;

private:
    Buffer* m_buffer = nullptr;
    Window* m_window = nullptr;
    NSOpenGLView* m_nsOpenGLView;
};

} // namespace OpenGL
} // namespace Display
} // namespace Simple
