//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/graphics/opengl/interop_gl.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class InteropGLHost : public InteropGL
{
public:
    InteropGLHost(GLuint a_pixelBufferId, void** a_bufferData);
    ~InteropGLHost() override;

    InteropGLHost(const InteropGLHost&) = delete;
    InteropGLHost& operator=(const InteropGLHost&) = delete;

    void Map(void** a_bufferData) override;
    void Unmap() override;

private:
    const GLuint m_pixelBufferId = 0;
};

//--------------------------------------------------------------
inline InteropGLHost::InteropGLHost(GLuint a_pixelBufferId,
                                    void** a_bufferData)
    : m_pixelBufferId(a_pixelBufferId)
{
    Map(a_bufferData);
}

//--------------------------------------------------------------
inline InteropGLHost::~InteropGLHost()
{
    Unmap();
}

//--------------------------------------------------------------
inline void InteropGLHost::Map(void** a_bufferData)
{
    // Map the pixel buffer to host memory.
    glBindBuffer(GL_ARRAY_BUFFER, m_pixelBufferId);
    *a_bufferData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

//--------------------------------------------------------------
inline void InteropGLHost::Unmap()
{
    // Unmap the pixel buffer from host memory.
    glBindBuffer(GL_ARRAY_BUFFER, m_pixelBufferId);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

} // namespace OpenGL
} // namespace Display
} // namespace Simple
