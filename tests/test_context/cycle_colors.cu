//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <simple/display/buffer.h>

#include <cuda_runtime.h>

using namespace Simple::Display;
using namespace std;

__constant__ float COLORS_FLOAT[4][4];
__constant__ uint8_t COLORS_UINT8[4][4];
__constant__ uint16_t COLORS_UINT16[4][4];

//--------------------------------------------------------------
template <typename BufferType>
__global__ void CycleColorsKernel(BufferType* a_pixelBuffer,
                                  uint32_t a_pixelWidth,
                                  uint32_t a_pixelHeight,
                                  uint32_t a_numChannels,
                                  uint32_t a_secondsElapsed)
{
    const uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    const uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= a_pixelWidth || y >= a_pixelHeight)
    {
        return;
    }

    const uint32_t topLeftIndex = a_secondsElapsed % 4;
    const int topRightIndex = (topLeftIndex == 3) ? 0 : topLeftIndex + 1;
    const int bottomLeftIndex = (topRightIndex == 3) ? 0 : topRightIndex + 1;
    const int bottomRightIndex = (bottomLeftIndex == 3) ? 0 : bottomLeftIndex + 1;

    const BufferType* colorTopLeft = nullptr;
    const BufferType* colorTopRight = nullptr;
    const BufferType* colorBottomLeft = nullptr;
    const BufferType* colorBottomRight = nullptr;
    if constexpr (std::is_same<BufferType, float>::value)
    {
        colorTopLeft = COLORS_FLOAT[topLeftIndex];
        colorTopRight = COLORS_FLOAT[topRightIndex];
        colorBottomLeft = COLORS_FLOAT[bottomLeftIndex];
        colorBottomRight = COLORS_FLOAT[bottomRightIndex];
    }
    else if constexpr (std::is_same<BufferType, uint8_t>::value)
    {
        colorTopLeft = COLORS_UINT8[topLeftIndex];
        colorTopRight = COLORS_UINT8[topRightIndex];
        colorBottomLeft = COLORS_UINT8[bottomLeftIndex];
        colorBottomRight = COLORS_UINT8[bottomRightIndex];
    }
    else if constexpr (std::is_same<BufferType, uint16_t>::value)
    {
        colorTopLeft = COLORS_UINT16[topLeftIndex];
        colorTopRight = COLORS_UINT16[topRightIndex];
        colorBottomLeft = COLORS_UINT16[bottomLeftIndex];
        colorBottomRight = COLORS_UINT16[bottomRightIndex];
    }
    else
    {
        static_assert(!std::is_same<BufferType, BufferType>::value, "Unsupported type");
    }

    const uint32_t quadrant = (x > (a_pixelWidth / 2)) +
                              (2 * (y > (a_pixelHeight / 2)));
    const BufferType* color = colorTopLeft;
    switch (quadrant)
    {
        case 0: color = colorBottomLeft; break;
        case 1: color = colorBottomRight; break;
        case 2: color = colorTopLeft; break;
        case 3: color = colorTopRight; break;
    }

    const uint32_t i = (x * a_numChannels) +
                       (y * a_pixelWidth * a_numChannels);
    for (uint32_t z = 0; z < a_numChannels; ++z)
    {
        a_pixelBuffer[i + z] = color[z];
    }
}

//--------------------------------------------------------------
template <typename BufferType>
extern void CycleColorsCuda(const Buffer& a_buffer,
                            float a_secondsElapsed)
{
    BufferType* pixelBuffer = a_buffer.GetData<BufferType, Buffer::Interop::CUDA>();
    if (!pixelBuffer)
    {
        return;
    }

    const uint32_t pixelWidth = a_buffer.GetWidth();
    const uint32_t pixelHeight = a_buffer.GetHeight();
    const uint32_t numChannels = Buffer::ChannelsPerPixel(a_buffer.GetFormat());

    dim3 blockDim(16, 16);
    dim3 gridDim((pixelWidth + blockDim.x - 1) / blockDim.x,
                 (pixelHeight + blockDim.y - 1) / blockDim.y);
    CycleColorsKernel<<<gridDim, blockDim>>>(pixelBuffer,
                                             pixelWidth,
                                             pixelHeight,
                                             numChannels,
                                             (int)a_secondsElapsed);
}

//--------------------------------------------------------------
extern void CycleColorsCuda(const float a_colors[4][4],
                            const Buffer& a_buffer,
                            float a_secondsElapsed)
{
    cudaMemcpyToSymbol(COLORS_FLOAT, a_colors, 16 * sizeof(float));
    CycleColorsCuda<float>(a_buffer, a_secondsElapsed);
}

//--------------------------------------------------------------
extern void CycleColorsCuda(const uint8_t a_colors[4][4],
                            const Buffer& a_buffer,
                            float a_secondsElapsed)
{
    cudaMemcpyToSymbol(COLORS_UINT8, a_colors, 16 * sizeof(uint8_t));
    CycleColorsCuda<uint8_t>(a_buffer, a_secondsElapsed);
}

//--------------------------------------------------------------
extern void CycleColorsCuda(const uint16_t a_colors[4][4],
                            const Buffer& a_buffer,
                            float a_secondsElapsed)
{
    cudaMemcpyToSymbol(COLORS_UINT16, a_colors, 16 * sizeof(uint16_t));
    CycleColorsCuda<uint16_t>(a_buffer, a_secondsElapsed);
}
