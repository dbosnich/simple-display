//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/context_implementation.h>

typedef struct __GLXcontextRec* GLXContext;

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class ContextLinuxGL : public Context::Implementation
{
public:
    ContextLinuxGL(const Context::Config& a_config);
    ~ContextLinuxGL() override;

    ContextLinuxGL(const ContextLinuxGL&) = delete;
    ContextLinuxGL& operator=(const ContextLinuxGL&) = delete;

protected:
    Buffer& GetBuffer() const override;
    Window* GetWindow() const override;

    void OnFrameStart() override;
    void OnFrameEnded() override;

private:
    Buffer* m_buffer = nullptr;
    Window* m_window = nullptr;
    GLXContext m_glxContext = nullptr;
};

} // namespace OpenGL
} // namespace Display
} // namespace Simple
