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
namespace Vulkan
{

struct PipelineContext;

//--------------------------------------------------------------
class ContextWin32VK : public Context::Implementation
{
public:
    ContextWin32VK(const Context::Config& a_config);
    ~ContextWin32VK() override;

    ContextWin32VK(const ContextWin32VK&) = delete;
    ContextWin32VK& operator=(const ContextWin32VK&) = delete;

protected:
    Buffer& GetBuffer() const override;
    Window* GetWindow() const override;

    void OnFrameStart() override;
    void OnFrameEnded() override;

private:
    Buffer* m_buffer = nullptr;
    Window* m_window = nullptr;
    PipelineContext* m_pipelineContext = nullptr;
};

} // namespace Vulkan
} // namespace Display
} // namespace Simple
