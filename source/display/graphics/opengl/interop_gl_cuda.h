//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/graphics/cuda/debug_cuda.h>
#include <display/graphics/opengl/interop_gl.h>

#include <cuda_gl_interop.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class InteropGLCuda : public InteropGL
{
public:
    InteropGLCuda(GLuint a_pixelBufferId, void** a_bufferData);
    ~InteropGLCuda() override;

    InteropGLCuda(const InteropGLCuda&) = delete;
    InteropGLCuda& operator=(const InteropGLCuda&) = delete;

    void Map(void** a_bufferData);
    void Unmap();

private:
    cudaGraphicsResource_t m_cudaResource = 0;
};

//--------------------------------------------------------------
inline InteropGLCuda::InteropGLCuda(GLuint a_pixelBufferId,
                                    void** a_bufferData)
{
    // Register the pixel buffer with CUDA.
    CUDA_ENSURE(cudaGraphicsGLRegisterBuffer(&m_cudaResource,
                                             a_pixelBufferId,
                                             cudaGraphicsMapFlagsWriteDiscard));

    // Map the pixel buffer to CUDA memory.
    Map(a_bufferData);
}

//--------------------------------------------------------------
inline InteropGLCuda::~InteropGLCuda()
{
    // Unmap the pixel buffer from CUDA memory.
    Unmap();

    // Deregister the pixel buffer from CUDA.
    CUDA_ENSURE(cudaGraphicsUnregisterResource(m_cudaResource));
}

//--------------------------------------------------------------
inline void InteropGLCuda::Map(void** a_bufferData)
{
    size_t size;
    CUDA_ENSURE(cudaGraphicsMapResources(1, &m_cudaResource, 0));
    CUDA_ENSURE(cudaGraphicsResourceGetMappedPointer(a_bufferData,
                                                     &size,
                                                     m_cudaResource));
}

//--------------------------------------------------------------
inline void InteropGLCuda::Unmap()
{
    CUDA_ENSURE(cudaGraphicsUnmapResources(1, &m_cudaResource, 0));
}

} // namespace OpenGL
} // namespace Display
} // namespace Simple
