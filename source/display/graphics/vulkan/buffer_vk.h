//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/buffer_implementation.h>
#include <display/graphics/vulkan/pipeline_vk.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace Vulkan
{

//--------------------------------------------------------------
class BufferVK : public Buffer::Implementation
{
public:
    BufferVK(const Buffer::Config& a_config,
             PipelineContext& a_pipelineContext);
    ~BufferVK() override;

    BufferVK(const BufferVK&) = delete;
    BufferVK& operator=(const BufferVK&) = delete;

protected:
    void Create(const Buffer::Config& a_config);
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

private:
    Buffer::Config m_config = Buffer::Config::Invalid();
    PipelineContext& m_pipelineContext;
    PipelineVK* m_pipeline = nullptr;
    void* m_data = nullptr;
};

//--------------------------------------------------------------
inline BufferVK::BufferVK(const Buffer::Config& a_config,
                          PipelineContext& a_pipelineContext)
    : m_pipelineContext(a_pipelineContext)
{
    Create(a_config);
}

//--------------------------------------------------------------
inline BufferVK::~BufferVK()
{
    Delete();
}

//--------------------------------------------------------------
inline void BufferVK::Create(const Buffer::Config& a_config)
{
    // Store the config.
    m_config = a_config;

    // Create the pipeline.
    assert(!m_data);
    assert(!m_pipeline);
    m_pipelineContext.bufferData = &m_data;
    m_pipeline = new PipelineVK(m_config,
                                m_pipelineContext);
}

//--------------------------------------------------------------
inline void BufferVK::Delete()
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
inline void BufferVK::Resize(const Buffer::Config& a_config)
{
    Delete();
    Create(a_config);
}

//--------------------------------------------------------------
inline void BufferVK::Render(uint32_t a_displayWidth,
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
inline void* BufferVK::GetData() const
{
    return m_data;
}

//--------------------------------------------------------------
inline uint32_t BufferVK::GetSize() const
{
    return Buffer::MinSizeBytes(m_config);
}

//--------------------------------------------------------------
inline uint32_t BufferVK::GetPitch() const
{
    return Buffer::MinPitchBytes(m_config);
}

//--------------------------------------------------------------
inline uint32_t BufferVK::GetWidth() const
{
    return m_config.width;
}

//--------------------------------------------------------------
inline uint32_t BufferVK::GetHeight() const
{
    return m_config.height;
}

//--------------------------------------------------------------
inline Buffer::Format BufferVK::GetFormat() const
{
    return m_config.format;
}

} // namespace Vulkan
} // namespace Display
} // namespace Simple
