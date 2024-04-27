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
namespace Vulkan
{

struct PipelineContext;

//--------------------------------------------------------------
class ContextMacOSVK : public Context::Implementation
{
public:
    ContextMacOSVK(const Context::Config& a_config);
    ~ContextMacOSVK() override;

    ContextMacOSVK(const ContextMacOSVK&) = delete;
    ContextMacOSVK& operator=(const ContextMacOSVK&) = delete;

protected:
    Buffer& GetBuffer() const override;
    Window* GetWindow() const override;

    void OnFrameStart() override;
    void OnFrameEnded() override;

private:
    Buffer* m_buffer = nullptr;
    Window* m_window = nullptr;
    MTKView* m_mtkView = nullptr;
    PipelineContext* m_pipelineContext = nullptr;
};

} // namespace Vulkan
} // namespace Display
} // namespace Simple
