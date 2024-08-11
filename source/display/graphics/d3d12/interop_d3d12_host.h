//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

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
class InteropD3D12Host : public InteropD3D12
{
public:
    InteropD3D12Host(Microsoft::WRL::ComPtr<ID3D12Resource>& o_sharedBuffer,
                     Microsoft::WRL::ComPtr<ID3D12Device>& a_device,
                     D3D12_RESOURCE_STATES& o_defaultResourceState,
                     void** a_bufferData,
                     UINT64 a_bufferSize);
    ~InteropD3D12Host() override;

    InteropD3D12Host(const InteropD3D12Host&) = delete;
    InteropD3D12Host& operator=(const InteropD3D12Host&) = delete;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource>& m_sharedBuffer;
};

//--------------------------------------------------------------
inline InteropD3D12Host::InteropD3D12Host(Microsoft::WRL::ComPtr<ID3D12Resource>& o_sharedBuffer,
                                          Microsoft::WRL::ComPtr<ID3D12Device>& a_device,
                                          D3D12_RESOURCE_STATES& o_defaultResourceState,
                                          void** a_bufferData,
                                          UINT64 a_bufferSize)
    : m_sharedBuffer(o_sharedBuffer)
{
    const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(a_bufferSize);
    o_defaultResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
    D3D12_ENSURE(a_device->CreateCommittedResource(&heapProperties,
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &resourceDesc,
                                                   o_defaultResourceState,
                                                   nullptr,
                                                   IID_PPV_ARGS(&m_sharedBuffer)));

    // Map the shared buffer.
    CD3DX12_RANGE readRange(0, 0);
    D3D12_ENSURE(m_sharedBuffer->Map(0, &readRange, a_bufferData));
}

//--------------------------------------------------------------
inline InteropD3D12Host::~InteropD3D12Host()
{
    m_sharedBuffer->Unmap(0, nullptr);
}

} // namespace DirectX
} // namespace Display
} // namespace Simple
