//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/buffer_implementation.h>
#include <display/graphics/metal/pipeline_mt.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace Metal
{

//--------------------------------------------------------------
class BufferMT : public Buffer::Implementation
{
public:
    BufferMT(const Buffer::Config& a_config,
             MTKView* a_metalView);
    ~BufferMT() override;

    BufferMT(const BufferMT&) = delete;
    BufferMT& operator=(const BufferMT&) = delete;

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
    PipelineMT* m_pipeline = nullptr;
    MTKView* m_metalView = nullptr;
    uint32_t m_alignedPitch = 0;
    uint32_t m_alignedSize = 0;
    void* m_data = nullptr;
};

//--------------------------------------------------------------
inline BufferMT::BufferMT(const Buffer::Config& a_config,
                          MTKView* a_metalView)
    : m_metalView(a_metalView)
{
    Create(a_config);
}

//--------------------------------------------------------------
inline BufferMT::~BufferMT()
{
    Delete();
}

//--------------------------------------------------------------
constexpr MTLPixelFormat GetMetalPixelFormat(const Buffer::Format& a_format)
{
    switch (a_format)
    {
        case Buffer::Format::RGBA_FLOAT: return MTLPixelFormatRGBA32Float;
        case Buffer::Format::RGBA_UINT8: return MTLPixelFormatRGBA8Uint;
        case Buffer::Format::RGBA_UINT16: return MTLPixelFormatRGBA16Uint;
        default: return MTLPixelFormatInvalid;
    }
}

//--------------------------------------------------------------
inline void BufferMT::Create(const Buffer::Config& a_config)
{
    assert(m_metalView);

    // Store the config.
    m_config = a_config;

    // Get the Metal pixel format.
    MTLPixelFormat format = GetMetalPixelFormat(m_config.format);
    assert(format != MTLPixelFormatInvalid);

    // Create the pipeline.
    assert(!m_data);
    assert(!m_pipeline);
    m_alignedPitch = Buffer::MinPitchBytes(m_config);
    m_alignedSize = Buffer::MinSizeBytes(m_config);
    m_pipeline = new PipelineMT(m_metalView,
                                m_data,
                                m_config.width,
                                m_config.height,
                                m_alignedPitch,
                                m_alignedSize,
                                format);
}

//--------------------------------------------------------------
inline void BufferMT::Delete()
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
inline void BufferMT::Resize(const Buffer::Config& a_config)
{
    Delete();
    Create(a_config);
}

//--------------------------------------------------------------
inline void BufferMT::Render(uint32_t a_displayWidth,
                             uint32_t a_displayHeight)
{
    // Render the pixel buffer.
    m_pipeline->Render(a_displayWidth, a_displayHeight);
}

//--------------------------------------------------------------
inline void* BufferMT::GetData() const
{
    return m_data;
}

//--------------------------------------------------------------
inline uint32_t BufferMT::GetSize() const
{
    return m_alignedSize;
}

//--------------------------------------------------------------
inline uint32_t BufferMT::GetPitch() const
{
    return m_alignedPitch;
}

//--------------------------------------------------------------
inline uint32_t BufferMT::GetWidth() const
{
    return m_config.width;
}

//--------------------------------------------------------------
inline uint32_t BufferMT::GetHeight() const
{
    return m_config.height;
}

//--------------------------------------------------------------
inline Buffer::Format BufferMT::GetFormat() const
{
    return m_config.format;
}

} // namespace Metal
} // namespace Display
} // namespace Simple
