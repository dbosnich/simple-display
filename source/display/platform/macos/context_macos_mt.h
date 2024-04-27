//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/context_implementation.h>

@class MTKView;

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace Metal
{

//--------------------------------------------------------------
class ContextMacOSMT : public Context::Implementation
{
public:
    ContextMacOSMT(const Context::Config& a_config);
    ~ContextMacOSMT() override;

    ContextMacOSMT(const ContextMacOSMT&) = delete;
    ContextMacOSMT& operator=(const ContextMacOSMT&) = delete;

protected:
    Buffer& GetBuffer() const override;
    Window* GetWindow() const override;

    void OnFrameStart() override;
    void OnFrameEnded() override;

private:
    Buffer* m_buffer = nullptr;
    Window* m_window = nullptr;
    MTKView* m_metalView = nullptr;
};

} // namespace Metal
} // namespace Display
} // namespace Simple
