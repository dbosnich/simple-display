//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#define NOMINMAX
#include <initguid.h>
#include "headers/d3d12.h"
#include "headers/d3dx12.h"
#include <D3Dcompiler.h>
#include <dxgi1_6.h>
#include <assert.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace DirectX
{

//--------------------------------------------------------------
class PipelineD3D12
{
    using Cfg = const Buffer::Config;
public:
    PipelineD3D12(HWND a_windowHandle,
                  void** a_bufferData,
                  Cfg& a_bufferConfig,
                  bool a_fullScreenState = false);
    ~PipelineD3D12();

    PipelineD3D12(const PipelineD3D12&) = delete;
    PipelineD3D12& operator=(const PipelineD3D12&) = delete;

    void Render(uint32_t a_displayWidth,
                uint32_t a_displayHeight);
    void WaitForFrameCompletion();

    uint32_t GetSwapChainWidth() const;
    uint32_t GetSwapChainHeight() const;
    bool GetCurrentFullScreenState() const;

private:
    // Minimal set of D3D constructs required
    // to render a pixel buffer to the screen.
    static constexpr uint32_t N = 2;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_graphicsRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_graphicsPipelineState;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_shaderResourceHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_renderTaregtHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargetViews[N];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_textureUploadHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_textureBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    CD3DX12_TEXTURE_COPY_LOCATION m_textureBufferCopyDest;
    CD3DX12_TEXTURE_COPY_LOCATION m_textureBufferCopySrc;
    CD3DX12_RESOURCE_BARRIER m_textureTransitionResource;
    CD3DX12_RESOURCE_BARRIER m_textureTransitionCopy;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    CD3DX12_VIEWPORT m_viewport = {};
    CD3DX12_RECT m_scissorRect = {};
    UINT m_rtDescriptorSize = 0;
    HANDLE m_fenceEvent = {};
    UINT64 m_fenceValue = 0;
    UINT m_frameIndex = 0;
};

//--------------------------------------------------------------
inline void AssertSucceeded(HRESULT a_result)
{
    (void)a_result;
    assert(SUCCEEDED(a_result));
}

//--------------------------------------------------------------
inline bool DoesAdapterSupportsD3D12(IDXGIAdapter1* adapter)
{
    DXGI_ADAPTER_DESC1 adapterDesc;
    adapter->GetDesc1(&adapterDesc);
    if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
    {
        // Don't select the Basic Render Driver adapter.
        return false;
    }

    return SUCCEEDED(D3D12CreateDevice(adapter,
                                       D3D_FEATURE_LEVEL_11_0,
                                       __uuidof(ID3D12Device),
                                       nullptr));
}

//--------------------------------------------------------------
inline PipelineD3D12::PipelineD3D12(HWND a_windowHandle,
                                    void** a_bufferData,
                                    Cfg& a_bufferConfig,
                                    bool a_fullScreenState)
{
    UINT factoryFlags = 0;
#if !defined(NDEBUG)
    // Enable debug layers.
    Microsoft::WRL::ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        debug->EnableDebugLayer();
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    // Create the factory.
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    AssertSucceeded(CreateDXGIFactory2(factoryFlags,
                                       IID_PPV_ARGS(&factory)));

    // Find the best GPU adapter.
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (uint32_t adapterIndex = 0;
             SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex,
                                                            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                            IID_PPV_ARGS(&adapter)));
             ++adapterIndex)
        {
            if (DoesAdapterSupportsD3D12(adapter.Get()))
            {
                break;
            }
            adapter = nullptr;
        }
    }

    // Fallback in case of no GPU.
    if (adapter.Get() == nullptr)
    {
        for (uint32_t adapterIndex = 0;
             SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter));
             ++adapterIndex)
        {
            if (DoesAdapterSupportsD3D12(adapter.Get()))
            {
                break;
            }
            adapter = nullptr;
        }
    }

    // Create the device.
    assert(adapter);
    AssertSucceeded(D3D12CreateDevice(adapter.Get(),
                                      D3D_FEATURE_LEVEL_11_0,
                                      IID_PPV_ARGS(&m_device)));

    // Create the command queue.
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        AssertSucceeded(m_device->CreateCommandQueue(&queueDesc,
                                                     IID_PPV_ARGS(&m_commandQueue)));
    }

    // Create the swap chain.
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = N;
        swapChainDesc.Width = 0; // Use the window output width.
        swapChainDesc.Height = 0; // Use the window output height.
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
        fullScreenDesc.Windowed = !a_fullScreenState;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        AssertSucceeded(factory->CreateSwapChainForHwnd(m_commandQueue.Get(),
                                                        a_windowHandle,
                                                        &swapChainDesc,
                                                        &fullScreenDesc,
                                                        nullptr,
                                                        &swapChain));
        AssertSucceeded(swapChain.As(&m_swapChain));
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Initialize the viewport and scissor rect.
        m_swapChain->GetDesc1(&swapChainDesc);
        m_viewport.TopLeftX = 0.0f;
        m_viewport.TopLeftY = 0.0f;
        m_viewport.Width = static_cast<FLOAT>(swapChainDesc.Width);
        m_viewport.Height = static_cast<FLOAT>(swapChainDesc.Height);

        m_scissorRect.top = 0;
        m_scissorRect.left = 0;
        m_scissorRect.right = swapChainDesc.Width;
        m_scissorRect.bottom = swapChainDesc.Height;
    }

    // Create the render target descriptor heap.
    {
        D3D12_DESCRIPTOR_HEAP_DESC renderTaregtHeapDesc = {};
        renderTaregtHeapDesc.NumDescriptors = N;
        renderTaregtHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        renderTaregtHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        AssertSucceeded(m_device->CreateDescriptorHeap(&renderTaregtHeapDesc,
                                                       IID_PPV_ARGS(&m_renderTaregtHeap)));
    }

    // Create the render target views.
    {
        m_rtDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_renderTaregtHeap->GetCPUDescriptorHandleForHeapStart());
        for (uint32_t n = 0; n < N; ++n)
        {
            AssertSucceeded(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargetViews[n])));
            m_device->CreateRenderTargetView(m_renderTargetViews[n].Get(), nullptr, rtHandle);
            rtHandle.Offset(1, m_rtDescriptorSize);
        }
    }

    // Create the graphics root signature.
    {
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,
                       D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        CD3DX12_ROOT_PARAMETER1 rootParams[1];
        rootParams[0].InitAsDescriptorTable(1, &ranges[0],
                                            D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParams),
                                   rootParams,
                                   1,
                                   &sampler,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Microsoft::WRL::ComPtr<ID3DBlob> error;
        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        AssertSucceeded(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                              D3D_ROOT_SIGNATURE_VERSION_1_0,
                                                              &signature,
                                                              &error));
        AssertSucceeded(m_device->CreateRootSignature(0,
                                                      signature->GetBufferPointer(),
                                                      signature->GetBufferSize(),
                                                      IID_PPV_ARGS(&m_graphicsRootSignature)));
    }

    // Create the graphics pipeline state.
    {
        const std::string shaderSource =
        R"(
            struct PSInput
            {
                float4 pos : SV_POSITION;
                float2 uv : TEXCOORD;
            };

            Texture2D g_texture : register(t0);
            SamplerState g_sampler : register(s0);

            PSInput VSMain(float4 pos : POSITION,
                           float4 uv : TEXCOORD)
            {
                PSInput result;
                result.pos = pos;
                result.uv = uv;
                return result;
            }

            float4 PSMain(PSInput input) : SV_TARGET
            {
                return g_texture.Sample(g_sampler, input.uv);
            }
        )";

        UINT compileFlags = 0;
    #if !defined(NDEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    #endif

        // Compile the vertex shader.
        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
        AssertSucceeded(D3DCompile(shaderSource.c_str(),
                                   shaderSource.size(),
                                   nullptr,
                                   nullptr,
                                   nullptr,
                                   "VSMain",
                                   "vs_5_0",
                                   compileFlags,
                                   0,
                                   &vertexShader,
                                   nullptr));

        // Compile the pixel shader.
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
        AssertSucceeded(D3DCompile(shaderSource.c_str(),
                                   shaderSource.size(),
                                   nullptr,
                                   nullptr,
                                   nullptr,
                                   "PSMain",
                                   "ps_5_0",
                                   compileFlags,
                                   0,
                                   &pixelShader,
                                   nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_graphicsRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        AssertSucceeded(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_graphicsPipelineState)));
    }

    // Create the vertex buffer.
    {
        struct Vertex
        {
            struct { float x, y, z; } pos;
            struct { float x, y; } uv;
        };
        const Vertex quadVertices[] =
        {
            { { -1.0f,-1.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
            { { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { 1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f } },
        };

        const UINT quadVerticesSize = sizeof(quadVertices);
        const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(quadVerticesSize);
        AssertSucceeded(m_device->CreateCommittedResource(&heapProperties,
                                                          D3D12_HEAP_FLAG_NONE,
                                                          &resourceDesc,
                                                          D3D12_RESOURCE_STATE_GENERIC_READ,
                                                          nullptr,
                                                          IID_PPV_ARGS(&m_vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);
        AssertSucceeded(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, quadVertices, sizeof(quadVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = quadVerticesSize;
    }

    // Create the texture.
    {
        DXGI_FORMAT bufferFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT shaderFormat = DXGI_FORMAT_UNKNOWN;
        switch (a_bufferConfig.format)
        {
            case Buffer::Format::RGBA_FLOAT:
            {
                bufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
                shaderFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
            break;
            case Buffer::Format::RGBA_UINT8:
            {
                bufferFormat = DXGI_FORMAT_R8G8B8A8_UINT;
                shaderFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            break;
            case Buffer::Format::RGBA_UINT16:
            {
                bufferFormat = DXGI_FORMAT_R16G16B16A16_UINT;
                shaderFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
            }
            break;
        }
        assert(bufferFormat != DXGI_FORMAT_UNKNOWN);
        assert(shaderFormat != DXGI_FORMAT_UNKNOWN);

        // Describe and create the Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = bufferFormat;
        textureDesc.Width = a_bufferConfig.width;
        textureDesc.Height = a_bufferConfig.height;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        const CD3DX12_HEAP_PROPERTIES heapPropertiesDefault(D3D12_HEAP_TYPE_DEFAULT);
        AssertSucceeded(m_device->CreateCommittedResource(&heapPropertiesDefault,
                                                          D3D12_HEAP_FLAG_NONE,
                                                          &textureDesc,
                                                          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                          nullptr,
                                                          IID_PPV_ARGS(&m_textureBuffer)));

        // Create the resource transitions.
        m_textureTransitionCopy = CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffer.Get(),
                                                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                                       D3D12_RESOURCE_STATE_COPY_DEST);
        m_textureTransitionResource = CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffer.Get(),
                                                                           D3D12_RESOURCE_STATE_COPY_DEST,
                                                                           D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // Create the texture upload buffer.
        const UINT64 bufferSize = Buffer::MinSizeBytes(a_bufferConfig);
        const CD3DX12_HEAP_PROPERTIES heapPropertiesUpload(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        AssertSucceeded(m_device->CreateCommittedResource(&heapPropertiesUpload,
                                                          D3D12_HEAP_FLAG_NONE,
                                                          &resourceDesc,
                                                          D3D12_RESOURCE_STATE_GENERIC_READ,
                                                          nullptr,
                                                          IID_PPV_ARGS(&m_textureUploadHeap)));

        // Map the texture upload buffer.
        CD3DX12_RANGE readRange(0, 0);
        AssertSucceeded(m_textureUploadHeap->Map(0, &readRange, a_bufferData));

        // Create the texture copy source and destination locations.
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresourceFootprint;
        subresourceFootprint.Offset = 0;
        subresourceFootprint.Footprint.Format = bufferFormat;
        subresourceFootprint.Footprint.Width = a_bufferConfig.width;
        subresourceFootprint.Footprint.Height = a_bufferConfig.height;
        subresourceFootprint.Footprint.Depth = 1;
        subresourceFootprint.Footprint.RowPitch = Buffer::MinPitchBytes(a_bufferConfig);
        m_textureBufferCopySrc = CD3DX12_TEXTURE_COPY_LOCATION(m_textureUploadHeap.Get(),
                                                               subresourceFootprint);
        m_textureBufferCopyDest = CD3DX12_TEXTURE_COPY_LOCATION(m_textureBuffer.Get());

        // Describe and create a shader resource heap for the texture.
        D3D12_DESCRIPTOR_HEAP_DESC shaderResourceHeapDesc = {};
        shaderResourceHeapDesc.NumDescriptors = 1;
        shaderResourceHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        shaderResourceHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        AssertSucceeded(m_device->CreateDescriptorHeap(&shaderResourceHeapDesc,
                                                       IID_PPV_ARGS(&m_shaderResourceHeap)));

        // Describe and create a shader resource view for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc = {};
        shaderResourceDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        shaderResourceDesc.Format = shaderFormat;
        shaderResourceDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        shaderResourceDesc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView(m_textureBuffer.Get(),
                                           &shaderResourceDesc,
                                           m_shaderResourceHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create the command allocator and command list.
    {
        AssertSucceeded(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                         IID_PPV_ARGS(&m_commandAlloc)));
        AssertSucceeded(m_device->CreateCommandList(0,
                                                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                    m_commandAlloc.Get(),
                                                    m_graphicsPipelineState.Get(),
                                                    IID_PPV_ARGS(&m_commandList)));
        AssertSucceeded(m_commandList->Close());
    }

    // Create synchronization objects.
    {
        AssertSucceeded(m_device->CreateFence(m_fenceValue,
                                              D3D12_FENCE_FLAG_NONE,
                                              IID_PPV_ARGS(&m_fence)));
        m_fenceValue++;

        m_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            AssertSucceeded(HRESULT_FROM_WIN32(GetLastError()));
        }

        WaitForFrameCompletion();
    }
}

//--------------------------------------------------------------
inline PipelineD3D12::~PipelineD3D12()
{
    AssertSucceeded(m_swapChain->SetFullscreenState(false, nullptr));
    WaitForFrameCompletion();
}

//--------------------------------------------------------------
inline void PipelineD3D12::Render(uint32_t a_displayWidth,
                                  uint32_t a_displayHeight)
{
    // Record all the commands needed to render the buffer.
    {
        // Reset the command allocator and command list.
        AssertSucceeded(m_commandAlloc->Reset());
        AssertSucceeded(m_commandList->Reset(m_commandAlloc.Get(),
                                             m_graphicsPipelineState.Get()));

        // Set the necessary graphics state.
        m_commandList->SetGraphicsRootSignature(m_graphicsRootSignature.Get());
        ID3D12DescriptorHeap* ppHeaps[] = { m_shaderResourceHeap.Get() };
        m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
        m_commandList->SetGraphicsRootDescriptorTable(0, m_shaderResourceHeap->GetGPUDescriptorHandleForHeapStart());

        // Set the viewport and scissor rect.
        (void)a_displayWidth;
        (void)a_displayHeight;
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        // Copy the pixel buffer to the texture.
        m_commandList->ResourceBarrier(1, &m_textureTransitionCopy);
        m_commandList->CopyTextureRegion(&m_textureBufferCopyDest, 0, 0, 0, &m_textureBufferCopySrc, nullptr);
        m_commandList->ResourceBarrier(1, &m_textureTransitionResource);

        // Render to the back buffer.
        const CD3DX12_RESOURCE_BARRIER transitionToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargetViews[m_frameIndex].Get(),
                                                                                                       D3D12_RESOURCE_STATE_PRESENT,
                                                                                                       D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &transitionToRenderTarget);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_renderTaregtHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtDescriptorSize);
        m_commandList->OMSetRenderTargets(1, &rtHandle, FALSE, nullptr);
        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_commandList->ClearRenderTargetView(rtHandle, clearColor, 0, nullptr);
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->DrawInstanced(4, 1, 0, 0);
        const CD3DX12_RESOURCE_BARRIER transitionToPresent = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargetViews[m_frameIndex].Get(),
                                                                                                  D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                                                  D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &transitionToPresent);

        // Close the command list.
        AssertSucceeded(m_commandList->Close());
    }

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    AssertSucceeded(m_swapChain->Present(1, 0));

    // Wait for the frame to complete.
    WaitForFrameCompletion();
}

//--------------------------------------------------------------
inline void PipelineD3D12::WaitForFrameCompletion()
{
    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    AssertSucceeded(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is completed.
    if (m_fence->GetCompletedValue() < fence)
    {
        AssertSucceeded(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        ::WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // Set the next frame index.
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

//--------------------------------------------------------------
inline uint32_t PipelineD3D12::GetSwapChainWidth() const
{
    return static_cast<uint32_t>(m_viewport.Width);
}

//--------------------------------------------------------------
inline uint32_t PipelineD3D12::GetSwapChainHeight() const
{
    return static_cast<uint32_t>(m_viewport.Height);
}

//--------------------------------------------------------------
inline bool PipelineD3D12::GetCurrentFullScreenState() const
{
    BOOL fullScreenState = false;
    m_swapChain->GetFullscreenState(&fullScreenState, nullptr);
    return fullScreenState;
}

} // namespace DirectX
} // namespace Display
} // namespace Simple
