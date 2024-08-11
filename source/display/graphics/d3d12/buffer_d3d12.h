//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/buffer_implementation.h>
#include <display/graphics/d3d12/pipeline_d3d12.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace DirectX
{

//--------------------------------------------------------------
class BufferD3D12 : public Buffer::Implementation
{
public:
    BufferD3D12(const Buffer::Config& a_config, HWND a_hwnd);
    ~BufferD3D12() override;

    BufferD3D12(const BufferD3D12&) = delete;
    BufferD3D12& operator=(const BufferD3D12&) = delete;

protected:
    void Create(const Buffer::Config& a_config,
                bool a_fullScreenState = false);
    void Delete();

    void Resize(const Buffer::Config& a_config) override;
    void Render(uint32_t a_displayWidth,
                uint32_t a_displayHeight) override;

    void* GetData() const override;
    uint32_t GetSize() const override;
    uint32_t GetPitch() const override;
    uint32_t GetWidth() const override;
    uint32_t GetHeight() const override;
    Buffer::Format GetFormat() const override;
    Buffer::Interop GetInterop() const override;

private:
    Buffer::Config m_config = Buffer::Config::Invalid();
    PipelineD3D12* m_pipeline = nullptr;
    void* m_data = nullptr;
    HWND m_hwnd = nullptr;
};

//--------------------------------------------------------------
inline BufferD3D12::BufferD3D12(const Buffer::Config& a_config,
                                HWND a_hwnd)
    : m_hwnd(a_hwnd)
{
    Create(a_config);
}

//--------------------------------------------------------------
inline BufferD3D12::~BufferD3D12()
{
    Delete();
}

//--------------------------------------------------------------
inline void BufferD3D12::Create(const Buffer::Config& a_config,
                                bool a_fullScreenState)
{
    // Store the config.
    m_config = a_config;

    // Create the pipeline.
    assert(m_hwnd);
    assert(!m_data);
    assert(!m_pipeline);
    m_pipeline = new PipelineD3D12(m_hwnd,
                                   &m_data,
                                   m_config,
                                   a_fullScreenState);
}

//--------------------------------------------------------------
inline void BufferD3D12::Delete()
{
    // Delete the pipeline.
    assert(m_pipeline);
    delete m_pipeline;
    m_pipeline = nullptr;
    m_data = nullptr;

    // Invalidate the config.
    m_config = Buffer::Config::Invalid();
}

//--------------------------------------------------------------
inline void BufferD3D12::Resize(const Buffer::Config& a_config)
{
    bool fullScreen = m_pipeline->GetCurrentFullScreenState();
    Delete();
    Create(a_config, fullScreen);
}

//--------------------------------------------------------------
inline void BufferD3D12::Render(uint32_t a_displayWidth,
                                uint32_t a_displayHeight)
{
    // Check if the pipeline needs to be resized...
    if (a_displayWidth != m_pipeline->GetSwapChainWidth() ||
        a_displayHeight != m_pipeline->GetSwapChainHeight())
    {
        // ...which is achieved simply by recreating it.
        Buffer::Config config = m_config;
        Resize(config);
    }

    // Render the pixel buffer.
    m_pipeline->Render(a_displayWidth, a_displayHeight);
}

//--------------------------------------------------------------
inline void* BufferD3D12::GetData() const
{
    return m_data;
}

//--------------------------------------------------------------
inline uint32_t BufferD3D12::GetSize() const
{
    return Buffer::MinSizeBytes(m_config);
}

//--------------------------------------------------------------
inline uint32_t BufferD3D12::GetPitch() const
{
    return Buffer::MinPitchBytes(m_config);
}

//--------------------------------------------------------------
inline uint32_t BufferD3D12::GetWidth() const
{
    return m_config.width;
}

//--------------------------------------------------------------
inline uint32_t BufferD3D12::GetHeight() const
{
    return m_config.height;
}

//--------------------------------------------------------------
inline Buffer::Format BufferD3D12::GetFormat() const
{
    return m_config.format;
}

//--------------------------------------------------------------
inline Buffer::Interop BufferD3D12::GetInterop() const
{
    return m_config.interop;
}

} // namespace DirectX
} // namespace Display
} // namespace Simple
