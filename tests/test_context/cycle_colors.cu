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

template <typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
__constant__ DataType COLORS[NumColors][ChannelsPerPixel];

//--------------------------------------------------------------
template<typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
__global__ void CycleColorsKernel(DataType* a_bufferData,
                                  uint32_t a_bufferWidth,
                                  uint32_t a_bufferHeight,
                                  uint32_t a_secondsElapsed)
{
    const uint32_t x = blockIdx.x * blockDim.x + threadIdx.x;
    const uint32_t y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= a_bufferWidth || y >= a_bufferHeight)
    {
        return;
    }

    const uint32_t topLeftIndex = a_secondsElapsed % NumColors;
    const uint32_t topRightIndex = (topLeftIndex == NumColors - 1) ? 0 : min(topLeftIndex + 1, NumColors);
    const uint32_t bottomLeftIndex = (topRightIndex == NumColors - 1) ? 0 : min(topRightIndex + 1, NumColors);
    const uint32_t bottomRightIndex = (bottomLeftIndex == NumColors - 1) ? 0 : min(bottomLeftIndex + 1, NumColors);

    const DataType* colorTopLeft = COLORS<DataType, ChannelsPerPixel, NumColors>[topLeftIndex];
    const DataType* colorTopRight = COLORS<DataType, ChannelsPerPixel, NumColors>[topRightIndex];
    const DataType* colorBottomLeft = COLORS<DataType, ChannelsPerPixel, NumColors>[bottomLeftIndex];
    const DataType* colorBottomRight = COLORS<DataType, ChannelsPerPixel, NumColors>[bottomRightIndex];

    const uint32_t quadrant = (x > (a_bufferWidth / 2)) +
                              (2 * (y > (a_bufferHeight / 2)));
    const DataType* color = colorTopLeft;
    switch (quadrant)
    {
        case 0: color = colorBottomLeft; break;
        case 1: color = colorBottomRight; break;
        case 2: color = colorTopLeft; break;
        case 3: color = colorTopRight; break;
    }

    const uint32_t i = (x * ChannelsPerPixel) +
                       (y * a_bufferWidth * ChannelsPerPixel);
    for (uint32_t z = 0; z < ChannelsPerPixel; ++z)
    {
        a_bufferData[i + z] = color[z];
    }
}

//--------------------------------------------------------------
template<typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
extern void CycleColorsCuda(const Buffer& a_buffer,
                            float a_secondsElapsed)
{
    const uint32_t bufferWidth = a_buffer.GetWidth();
    const uint32_t bufferHeight = a_buffer.GetHeight();
    DataType* bufferData = a_buffer.GetData<DataType, Buffer::Interop::CUDA>();
    if (!bufferData || !bufferWidth || !bufferHeight || !bufferData)
    {
        return;
    }

    dim3 blockDim(16, 16);
    dim3 gridDim((bufferWidth + blockDim.x - 1) / blockDim.x,
                 (bufferHeight + blockDim.y - 1) / blockDim.y);
    CycleColorsKernel<DataType, ChannelsPerPixel, NumColors><<<gridDim, blockDim>>>(bufferData,
                                                                                    bufferWidth,
                                                                                    bufferHeight,
                                                                                    (uint32_t)a_secondsElapsed);
}

//--------------------------------------------------------------
template<typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
extern void CycleColorsCuda(const DataType a_colors[NumColors][ChannelsPerPixel],
                            const Buffer& a_buffer,
                            float a_secondsElapsed)
{
    // Note: This is not thread safe, but it avoids the calling
    // code needing CUDA specific code to pass a device pointer.
    cudaMemcpyToSymbol(COLORS<DataType, ChannelsPerPixel, NumColors>,
                       a_colors,
                       ChannelsPerPixel * NumColors * sizeof(DataType));
    CycleColorsCuda<DataType, ChannelsPerPixel, NumColors>(a_buffer, a_secondsElapsed);
}

//--------------------------------------------------------------
template void CycleColorsCuda<float, 4, 4>(const float a_colors[4][4],
                                           const Buffer& a_buffer,
                                           float a_secondsElapsed);

//--------------------------------------------------------------
template void CycleColorsCuda<uint8_t, 4, 4>(const uint8_t a_colors[4][4],
                                             const Buffer& a_buffer,
                                             float a_secondsElapsed);

//--------------------------------------------------------------
template void CycleColorsCuda<uint16_t, 4, 4>(const uint16_t a_colors[4][4],
                                              const Buffer& a_buffer,
                                              float a_secondsElapsed);
