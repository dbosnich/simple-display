//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/graphics/opengl/buffer_gl.h>
#include <display/graphics/opengl/interop_gl_host.h>
#ifdef CUDA_SUPPORTED
#   include <display/graphics/opengl/interop_gl_cuda.h>
#endif // CUDA_SUPPORTED

#include <assert.h>
#include <string>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class BufferGLCore : public BufferGL
{
public:
    BufferGLCore(const Buffer::Config& a_config);
    ~BufferGLCore() override;

    BufferGLCore(const BufferGLCore&) = delete;
    BufferGLCore& operator=(const BufferGLCore&) = delete;

protected:
    void Create(const Buffer::Config& a_config);
    void Delete();

    void Resize(const Buffer::Config& a_config) override;
    void Render(uint32_t a_displayWidth,
                uint32_t a_displayHeight) override;

private:
    GLuint m_programId = 0;
    GLuint m_textureId = 0;
    GLuint m_pixelBufferId = 0;
    GLuint m_vertexArrayId = 0;
    GLuint m_vertexBufferId = 0;
    GLenum m_glPixelDataType = 0;
    GLenum m_glPixelDataFormat = 0;
    InteropGL* m_pixelBufferInterop = 0;
};

//--------------------------------------------------------------
void InitializeProgram(GLuint programId);
void InitializeTexture(GLuint textureId);
void InitializeVertices(GLuint bufferId, GLuint arrayId);

//--------------------------------------------------------------
inline BufferGLCore::BufferGLCore(const Buffer::Config& a_config)
{
    // Create an OpenGL program to render a texture for display.
    m_programId = glCreateProgram();
    InitializeProgram(m_programId);

    // Create the texture that will be rendered to the display.
    glGenTextures(1, &m_textureId);
    InitializeTexture(m_textureId);

    // Create the vertex buffer that will be used to map the
    // texture to a quad that is scaled to fill the display,
    // along with the vertex array object needed to draw it.
    glGenBuffers(1, &m_vertexBufferId);
    glGenVertexArrays(1, &m_vertexArrayId);
    InitializeVertices(m_vertexBufferId, m_vertexArrayId);

    // Create pixel buffer that will be rendered to the display.
    Create(a_config);
}

//--------------------------------------------------------------
inline BufferGLCore::~BufferGLCore()
{
    // Delete the pixel buffer.
    Delete();

    // Delete the vertex array.
    glDeleteVertexArrays(1, &m_vertexArrayId);

    // Delete the vertex buffer.
    glDeleteBuffers(1, &m_vertexBufferId);

    // Delete the texture.
    glDeleteTextures(1, &m_textureId);

    // Delete the program.
    glDeleteProgram(m_programId);
}

//--------------------------------------------------------------
inline void BufferGLCore::Create(const Buffer::Config& a_config)
{
    assert(!m_data);
    assert(!m_pixelBufferInterop);

    // Store the config.
    m_config = a_config;

    // Store the GL pixel data values.
    m_glPixelDataType = GetGLPixelDataType(m_config.format);
    m_glPixelDataFormat = GetGLPixelDataFormat(m_config.format);

    // Create the texture image.
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GetGLInternalPixelFormat(m_config.format),
                 m_config.width,
                 m_config.height,
                 0,
                 m_glPixelDataFormat,
                 m_glPixelDataType,
                 0);

    // Create the pixel buffer.
    glGenBuffers(1, &m_pixelBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, m_pixelBufferId);
    glBufferData(GL_ARRAY_BUFFER,
                 Buffer::MinSizeBytes(m_config),
                 nullptr,
                 GL_STREAM_DRAW);

    // Create the appropriate interop to map the pixel buffer.
    if (m_config.interop == Buffer::Interop::HOST)
    {
        m_pixelBufferInterop = new InteropGLHost(m_pixelBufferId,
                                                 &m_data);
    }
    else if (m_config.interop == Buffer::Interop::CUDA)
    {
    #ifdef CUDA_SUPPORTED
        m_pixelBufferInterop = new InteropGLCuda(m_pixelBufferId,
                                                 &m_data);
    #endif // CUDA_SUPPORTED
    }
    assert(m_pixelBufferInterop);
    assert(m_data);
}

//--------------------------------------------------------------
inline void BufferGLCore::Delete()
{
    assert(m_pixelBufferInterop);
    delete m_pixelBufferInterop;
    m_pixelBufferInterop = nullptr;

    assert(m_data);
    m_data = nullptr;

    // Delete the pixel buffer.
    glDeleteBuffers(1, &m_pixelBufferId);
    m_pixelBufferId = 0;

    // Clear the GL pixel data values.
    m_glPixelDataType = 0;
    m_glPixelDataFormat = 0;

    // Invalidate the config.
    m_config = Buffer::Config::Invalid();
}

//--------------------------------------------------------------
inline void BufferGLCore::Resize(const Buffer::Config& a_config)
{
    Delete();
    Create(a_config);
}

//--------------------------------------------------------------
inline void BufferGLCore::Render(uint32_t a_displayWidth,
                                 uint32_t a_displayHeight)
{
    m_pixelBufferInterop->Unmap();
    m_data = nullptr;

    // Copy the pixel buffer to the texture.
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pixelBufferId);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    m_config.width,
                    m_config.height,
                    m_glPixelDataFormat,
                    m_glPixelDataType,
                    0);

    // Clear the display and set the viewport size.
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, a_displayWidth, a_displayHeight);

    // Draw the texture onto the quad.
    glUseProgram(m_programId);
    glBindVertexArray(m_vertexArrayId);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_pixelBufferInterop->Map(&m_data);
    assert(m_data);
}

//--------------------------------------------------------------
inline void CompileShader(GLuint shaderId,
                          const std::string& source)
{
    assert(shaderId);

    const GLchar* sourceData = source.data();
    glShaderSource(shaderId, 1, &sourceData, nullptr);
    glCompileShader(shaderId);
}

//--------------------------------------------------------------
inline void InitializeProgram(GLuint programId)
{
    assert(programId);

    // Create and compile the vertex shader.
    const std::string vertShaderSource =
    R"(
        #version 410 core

        layout(location = 0) in vec3 vertexPos;
        out vec2 uv;

        void main()
        {
	        gl_Position = vec4(vertexPos, 1);
            uv = vec2((vertexPos.x * 0.5) + 0.5,
                      (vertexPos.y * 0.5) + 0.5);
        }
    )";
    const GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    CompileShader(vertShader, vertShaderSource);

    // Create and compile the fragment shader.
    const std::string fragShaderSource =
    R"(
        #version 410 core

        in vec2 uv;
        out vec3 color;
        uniform sampler2D texSampler;

        void main()
        {
            color = texture(texSampler, uv).xyz;
        }
    )";
    const GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    CompileShader(fragShader, fragShaderSource);

    // Attach the shaders and link the program.
    glAttachShader(programId, vertShader);
    glAttachShader(programId, fragShader);
    glLinkProgram(programId);

    // Detach the shaders and then delete them.
    glDetachShader(programId, fragShader);
    glDetachShader(programId, vertShader);
    glDeleteShader(fragShader);
    glDeleteShader(vertShader);
}

//--------------------------------------------------------------
inline void InitializeTexture(GLuint textureId)
{
    assert(textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//--------------------------------------------------------------
inline void InitializeVertices(GLuint bufferId, GLuint arrayId)
{
    assert(bufferId);

    const GLfloat quadVertexBuffer[] = { -1.0f, -1.0f, 0.0f,
                                          1.0f, -1.0f, 0.0f,
                                         -1.0f,  1.0f, 0.0f,
                                         -1.0f,  1.0f, 0.0f,
                                          1.0f, -1.0f, 0.0f,
                                          1.0f,  1.0f, 0.0f };
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(quadVertexBuffer),
                 quadVertexBuffer,
                 GL_STATIC_DRAW);

    glBindVertexArray(arrayId);
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
}

} // namespace OpenGL
} // namespace Display
} // namespace Simple
