//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/context_implementation.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace DirectX
{

//--------------------------------------------------------------
class ContextWin32DX : public Context::Implementation
{
public:
    ContextWin32DX(const Context::Config& a_config);
    ~ContextWin32DX() override;

    ContextWin32DX(const ContextWin32DX&) = delete;
    ContextWin32DX& operator=(const ContextWin32DX&) = delete;

protected:
    Buffer& GetBuffer() const override;
    Window* GetWindow() const override;

    void OnFrameStart() override;
    void OnFrameEnded() override;

private:
    Buffer* m_buffer = nullptr;
    Window* m_window = nullptr;
};

} // namespace DirectX
} // namespace Display
} // namespace Simple
