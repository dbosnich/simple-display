//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <string>

#import <MetalKit/MetalKit.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace Metal
{

//--------------------------------------------------------------
class PipelineMT
{
public:
    PipelineMT(MTKView* a_mtkView,
               void*& a_bufferData,
               uint32_t a_bufferWidth,
               uint32_t a_bufferHeight,
               uint32_t& o_bufferRowPitch,
               uint32_t& o_bufferSizeBytes,
               MTLPixelFormat a_bufferFormat);
    ~PipelineMT();

    PipelineMT(const PipelineMT&) = delete;
    PipelineMT& operator=(const PipelineMT&) = delete;

    void Render(uint32_t a_displayWidth,
                uint32_t a_displayHeight);

private:
    MTKView* m_metalView;

    id<MTLCommandQueue> m_commandQueue;
    id<MTLRenderPipelineState> m_renderPipelineState;

    id<MTLTexture> m_texture;
    id<MTLBuffer> m_textureBuffer;

    id<MTLBuffer> m_vertexBuffer;
    uint32_t m_numVertices;
};

//--------------------------------------------------------------
inline PipelineMT::PipelineMT(MTKView* a_mtkView,
                              void*& a_bufferData,
                              uint32_t a_bufferWidth,
                              uint32_t a_bufferHeight,
                              uint32_t& o_bufferRowPitch,
                              uint32_t& o_bufferSizeBytes,
                              MTLPixelFormat a_bufferFormat)
{
    // Store the metal view.
    m_metalView = a_mtkView;
    assert(m_metalView);

    // Get the Metal device.
    id<MTLDevice> device = m_metalView.device;
    assert(device);

    // Align the buffer row pitch if necessary.
    NSUInteger minAlignment = [device minimumLinearTextureAlignmentForPixelFormat: a_bufferFormat];
    NSUInteger remainder = o_bufferRowPitch % minAlignment;
    if (remainder)
    {
        o_bufferRowPitch += (minAlignment - remainder);
        o_bufferSizeBytes = o_bufferRowPitch * a_bufferHeight;
    }

    // Create the texture buffer.
    m_textureBuffer = [device newBufferWithLength: o_bufferSizeBytes
                                          options: MTLResourceStorageModeShared];
    assert(m_textureBuffer);

    // Map the buffer data.
    a_bufferData = m_textureBuffer.contents;

    // Describe the texture.
    MTLTextureDescriptor* textureDescriptor = [[MTLTextureDescriptor alloc] init];
    textureDescriptor.pixelFormat = a_bufferFormat;
    textureDescriptor.width = a_bufferWidth;
    textureDescriptor.height = a_bufferHeight;
    textureDescriptor.textureType = MTLTextureType2D;
    textureDescriptor.resourceOptions = MTLResourceStorageModeShared;

    // Create the texture.
    m_texture = [m_textureBuffer newTextureWithDescriptor: textureDescriptor
                                                   offset: 0
                                              bytesPerRow: o_bufferRowPitch];

    // Define vertices to render a quad over the entire display.
    struct Vertex { vector_float2 pos, uv; };
    static constexpr Vertex QuadVertices[] =
    {
        // Pixel positions and uv coordinates.
        { {  1.0f,  -1.0f },  { 1.f, 1.f } },
        { { -1.0f,  -1.0f },  { 0.f, 1.f } },
        { { -1.0f,   1.0f },  { 0.f, 0.f } },

        { {  1.0f,  -1.0f },  { 1.f, 1.f } },
        { { -1.0f,   1.0f },  { 0.f, 0.f } },
        { {  1.0f,   1.0f },  { 1.f, 0.f } },
    };

    // Create the vertex buffer.
    m_vertexBuffer = [device newBufferWithBytes: QuadVertices
                                         length: sizeof(QuadVertices)
                                        options: MTLResourceStorageModeShared];
    m_numVertices = sizeof(QuadVertices) / sizeof(Vertex);

    // Inline shader source.
    const std::string shaderSource =
    R"(
        #include <metal_stdlib>

        using namespace metal;

        struct Vertex
        {
            vector_float2 pos;
            vector_float2 uv;
        };

        struct VertexData
        {
            float4 position [[position]];
            float2 textureUV;

        };

        vertex VertexData
        vertexShader(uint vertexID [[ vertex_id ]],
                     constant Vertex* vertexArray [[ buffer(0) ]])

        {
            VertexData out;
            float2 positionXY = vertexArray[vertexID].pos.xy;
            out.position = vector_float4(positionXY, 0.0, 1.0);
            out.textureUV = vertexArray[vertexID].uv;
            return out;
        }

        fragment float4
        fragmentShader(VertexData in [[stage_in]],
                       texture2d<half> colorTexture [[ texture(0) ]])
        {
            constexpr sampler textureSampler (mag_filter::linear,
                                              min_filter::linear);
            return float4(colorTexture.sample(textureSampler, in.textureUV));
        }
    )";

    // Load the shaders.
    NSString* shaderSourceString = [NSString stringWithUTF8String: shaderSource.c_str()];
    id<MTLLibrary> library = [device newLibraryWithSource: shaderSourceString
                                                  options: nil
                                                    error: nil];
    assert(library);

    // Get the vertex shader function.
    id<MTLFunction> vertexFunction = [library newFunctionWithName: @"vertexShader"];
    assert(vertexFunction);

    // Get the fragment shader function.
    id<MTLFunction> fragmentFunction = [library newFunctionWithName : @"fragmentShader"];
    assert(fragmentFunction);

    // Describe the render pipeline.
    MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.label = @"Texturing Pipeline";
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.colorAttachments[0].pixelFormat = m_metalView.colorPixelFormat;

    // Create the render pipeline state.
    m_renderPipelineState = [device newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                                   error: nil];
    assert(m_renderPipelineState);

    // Create the command queue.
    m_commandQueue = [device newCommandQueue];
    assert(m_commandQueue);
}

//--------------------------------------------------------------
inline PipelineMT::~PipelineMT()
{
    // Release the command queue.
    [m_commandQueue release];
    m_commandQueue = nullptr;

    // Release the render pipeline.
    [m_renderPipelineState release];
    m_renderPipelineState = nullptr;

    // Release the vertex buffer.
    [m_vertexBuffer release];
    m_vertexBuffer = nullptr;

    // Release the texture.
    [m_texture release];
    m_texture = nullptr;

    // Release the texture buffer.
    [m_textureBuffer release];
    m_textureBuffer = nullptr;
}

//--------------------------------------------------------------
inline void PipelineMT::Render(uint32_t a_displayWidth,
                               uint32_t a_displayHeight)
{
    (void)a_displayWidth;
    (void)a_displayHeight;

    // Create a new command buffer for each render pass.
    id<MTLCommandBuffer> commandBuffer = [m_commandQueue commandBuffer];
    commandBuffer.label = @"SimpleDisplayRenderCommandBuffer";

    // Record all the commands needed to render the pixel buffer.
    MTLRenderPassDescriptor* renderPassDescriptor = m_metalView.currentRenderPassDescriptor;
    if (renderPassDescriptor != nil)
    {
        // Start the render command encoder.
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor: renderPassDescriptor];
        renderEncoder.label = @"SimpleDisplayRenderEncoder";

        // Set the pipeline state.
        [renderEncoder setRenderPipelineState: m_renderPipelineState];

        // Set the vertex buffer.
        [renderEncoder setVertexBuffer: m_vertexBuffer
                                offset: 0
                               atIndex: 0];

        // Set the texture.
        [renderEncoder setFragmentTexture: m_texture
                                  atIndex: 0];

        // Draw the vertices.
        [renderEncoder drawPrimitives: MTLPrimitiveTypeTriangle
                          vertexStart: 0
                          vertexCount: m_numVertices];

        // End the render command encoder.
        [renderEncoder endEncoding];
    }

    // Present the current drawable
    [commandBuffer presentDrawable: m_metalView.currentDrawable];

    // Commit the command buffer.
    [commandBuffer commit];
}

} // namespace Metal
} // namespace Display
} // namespace Simple
