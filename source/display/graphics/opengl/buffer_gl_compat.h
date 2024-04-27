//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/graphics/opengl/buffer_gl_common.h>

#include <assert.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class BufferGLCompat : public BufferGLCommon
{
public:
    BufferGLCompat(const Buffer::Config& a_config);
    ~BufferGLCompat() override;

    BufferGLCompat(const BufferGLCompat&) = delete;
    BufferGLCompat& operator=(const BufferGLCompat&) = delete;

protected:
    void Create(const Buffer::Config& a_config);
    void Delete();

    void Resize(const Buffer::Config& a_config) override;
    void Render(uint32_t a_displayWidth,
                uint32_t a_displayHeight) override;

private:
    GLenum m_glPixelDataType = 0;
    GLenum m_glPixelDataFormat = 0;
};

//--------------------------------------------------------------
inline BufferGLCompat::BufferGLCompat(const Buffer::Config& a_config)
{
    // Create the pixel buffer data.
    Create(a_config);
}

//--------------------------------------------------------------
inline BufferGLCompat::~BufferGLCompat()
{
    // Delete the pixel buffer data.
    Delete();
}

//--------------------------------------------------------------
inline void BufferGLCompat::Create(const Buffer::Config& a_config)
{
    assert(!m_data);

    // Store the config.
    m_config = a_config;

    // Store the GL pixel data values.
    m_glPixelDataType = GetGLPixelDataType(m_config.format);
    m_glPixelDataFormat = GetGLPixelDataFormat(m_config.format);

    // Allocate the pixel data memory.
    const uint32_t sizeBytes = Buffer::MinSizeBytes(m_config);
    m_data = ::operator new(sizeBytes);
    memset(m_data, 0, sizeBytes);
}

//--------------------------------------------------------------
inline void BufferGLCompat::Delete()
{
    // Deallocate the pixel data memory.
    assert(m_data);
    ::operator delete(m_data);
    m_data = nullptr;

    // Clear the GL pixel data values.
    m_glPixelDataType = 0;
    m_glPixelDataFormat = 0;

    // Invalidate the config.
    m_config = Buffer::Config::Invalid();
}

//--------------------------------------------------------------
inline void BufferGLCompat::Resize(const Buffer::Config& a_config)
{
    Delete();
    Create(a_config);
}

//--------------------------------------------------------------
inline void BufferGLCompat::Render(uint32_t a_displayWidth,
                                   uint32_t a_displayHeight)
{
    // Clear the display.
    glClear(GL_COLOR_BUFFER_BIT);

    // Ensure the entire display is filled.
    const float xZoomFactor = static_cast<float>(a_displayWidth) /
                              static_cast<float>(m_config.width);
    const float yZoomFactor = static_cast<float>(a_displayHeight) /
                              static_cast<float>(m_config.height);
    glPixelZoom(xZoomFactor, yZoomFactor);

    // Draw the pixels onto the display.
    glDrawPixels(m_config.width,
                 m_config.height,
                 m_glPixelDataFormat,
                 m_glPixelDataType,
                 m_data);
}

} // namespace OpenGL
} // namespace Display
} // namespace Simple
