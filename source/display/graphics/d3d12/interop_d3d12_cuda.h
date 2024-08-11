//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/graphics/cuda/debug_cuda.h>
#include <display/graphics/d3d12/debug_d3d12.h>
#include <display/graphics/d3d12/interop_d3d12.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace DirectX
{

//--------------------------------------------------------------
class InteropD3D12Cuda : public InteropD3D12
{
public:
    InteropD3D12Cuda(Microsoft::WRL::ComPtr<ID3D12Resource>& o_sharedBuffer,
                     Microsoft::WRL::ComPtr<ID3D12Device>& a_device,
                     D3D12_RESOURCE_STATES& o_defaultResourceState,
                     void** a_bufferData,
                     UINT64 a_bufferSize);
    ~InteropD3D12Cuda() override;

    InteropD3D12Cuda(const InteropD3D12Cuda&) = delete;
    InteropD3D12Cuda& operator=(const InteropD3D12Cuda&) = delete;

private:
    void** m_bufferData = nullptr;
    cudaExternalMemory_t m_cudaExternalMemory = nullptr;
};

//--------------------------------------------------------------
inline InteropD3D12Cuda::InteropD3D12Cuda(Microsoft::WRL::ComPtr<ID3D12Resource>& o_sharedBuffer,
                                          Microsoft::WRL::ComPtr<ID3D12Device>& a_device,
                                          D3D12_RESOURCE_STATES& o_defaultResourceState,
                                          void** a_bufferData,
                                          UINT64 a_bufferSize)
    : m_bufferData(a_bufferData)
{
    // Create the shared buffer.
    const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(a_bufferSize,
                                                                             D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    o_defaultResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    D3D12_ENSURE(a_device->CreateCommittedResource(&heapProperties,
                                                   D3D12_HEAP_FLAG_SHARED,
                                                   &resourceDesc,
                                                   o_defaultResourceState,
                                                   nullptr,
                                                   IID_PPV_ARGS(&o_sharedBuffer)));

    // Create the shared memory handle.
    HANDLE sharedHandle;
    D3D12_ENSURE(a_device->CreateSharedHandle(o_sharedBuffer.Get(),
                                              nullptr,
                                              GENERIC_ALL,
                                              nullptr,
                                              &sharedHandle));

    // Describe and import the CUDA memory handle.
    cudaExternalMemoryHandleDesc externalMemoryHandleDesc;
    memset(&externalMemoryHandleDesc, 0, sizeof(externalMemoryHandleDesc));
    externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeD3D12Resource;
    externalMemoryHandleDesc.flags = cudaExternalMemoryDedicated;
    externalMemoryHandleDesc.handle.win32.handle = sharedHandle;
    externalMemoryHandleDesc.size = a_bufferSize;
    CUDA_ENSURE(cudaImportExternalMemory(&m_cudaExternalMemory,
                                         &externalMemoryHandleDesc));

    // Map the CUDA memory buffer.
    cudaExternalMemoryBufferDesc externalMemoryBufferDesc;
    memset(&externalMemoryBufferDesc, 0, sizeof(externalMemoryBufferDesc));
    externalMemoryBufferDesc.flags = 0;
    externalMemoryBufferDesc.offset = 0;
    externalMemoryBufferDesc.size = a_bufferSize;
    CUDA_ENSURE(cudaExternalMemoryGetMappedBuffer(a_bufferData,
                                                  m_cudaExternalMemory,
                                                  &externalMemoryBufferDesc));

    // Close the shared memory handle.
    CloseHandle(sharedHandle);
}

//--------------------------------------------------------------
inline InteropD3D12Cuda::~InteropD3D12Cuda()
{
    // Free the mapped buffer data.
    CUDA_ENSURE(cudaFree(*m_bufferData));

    // Destroy the external CUDA memory handle.
    CUDA_ENSURE(cudaDestroyExternalMemory(m_cudaExternalMemory));
}

} // namespace DirectX
} // namespace Display
} // namespace Simple
