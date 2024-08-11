//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/buffer_implementation.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class BufferGL : public Buffer::Implementation
{
public:
    BufferGL() = default;
    ~BufferGL() override = default;

    BufferGL(const BufferGL&) = delete;
    BufferGL& operator=(const BufferGL&) = delete;

protected:
    void* GetData() const override;
    uint32_t GetSize() const override;
    uint32_t GetPitch() const override;
    uint32_t GetWidth() const override;
    uint32_t GetHeight() const override;
    Buffer::Format GetFormat() const override;
    Buffer::Interop GetInterop() const override;

protected:
    Buffer::Config m_config = Buffer::Config::Invalid();
    void* m_data = nullptr;
};

//--------------------------------------------------------------
inline void* BufferGL::GetData() const
{
    return m_data;
}

//--------------------------------------------------------------
inline uint32_t BufferGL::GetSize() const
{
    return Buffer::MinSizeBytes(m_config);
}

//--------------------------------------------------------------
inline uint32_t BufferGL::GetPitch() const
{
    return Buffer::MinPitchBytes(m_config);
}

//--------------------------------------------------------------
inline uint32_t BufferGL::GetWidth() const
{
    return m_config.width;
}

//--------------------------------------------------------------
inline uint32_t BufferGL::GetHeight() const
{
    return m_config.height;
}

//--------------------------------------------------------------
inline Buffer::Format BufferGL::GetFormat() const
{
    return m_config.format;
}

//--------------------------------------------------------------
inline Buffer::Interop BufferGL::GetInterop() const
{
    return m_config.interop;
}

//--------------------------------------------------------------
constexpr GLenum GetGLPixelDataType(Buffer::Format a_format)
{
    switch (a_format)
    {
        case Buffer::Format::RGBA_FLOAT: return GL_FLOAT;
        case Buffer::Format::RGBA_UINT8: return GL_UNSIGNED_BYTE;
        case Buffer::Format::RGBA_UINT16: return GL_UNSIGNED_SHORT;
        default: return 0;
    }
}

//--------------------------------------------------------------
constexpr GLenum GetGLPixelDataFormat(Buffer::Format a_format)
{
    switch (a_format)
    {
        case Buffer::Format::RGBA_FLOAT: return GL_RGBA;
        case Buffer::Format::RGBA_UINT8: return GL_RGBA;
        case Buffer::Format::RGBA_UINT16: return GL_RGBA;
        default: return 0;
    }
}

//--------------------------------------------------------------
constexpr GLint GetGLInternalPixelFormat(Buffer::Format a_format)
{
    switch (a_format)
    {
        case Buffer::Format::RGBA_FLOAT: return GL_RGBA32F;
        case Buffer::Format::RGBA_UINT8: return GL_RGBA8;
        case Buffer::Format::RGBA_UINT16: return GL_RGBA16;
        default: return 0;
    }
}

} // namespace OpenGL
} // namespace Display
} // namespace Simple
