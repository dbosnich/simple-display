//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/graphics/vulkan/debug_vk.h>
#include <display/graphics/vulkan/interop_vk.h>
#include <display/graphics/vulkan/pipeline_vk.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace Vulkan
{

//--------------------------------------------------------------
class InteropVKHost : public InteropVK
{
public:
    InteropVKHost(PipelineVK& a_pipeline);
    ~InteropVKHost() override;

    InteropVKHost(const InteropVKHost&) = delete;
    InteropVKHost& operator=(const InteropVKHost&) = delete;

private:
    PipelineVK& m_pipeline;
};

//--------------------------------------------------------------
inline InteropVKHost::InteropVKHost(PipelineVK& a_pipeline)
    : m_pipeline(a_pipeline)
{
    // Create the shared buffer.
    const VkDeviceSize sharedBufferSize = Buffer::MinSizeBytes(m_pipeline.m_bufferConfig);
    m_pipeline.CreateBuffer(m_pipeline.m_sharedBuffer,
                            m_pipeline.m_sharedBufferMemory,
                            sharedBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Map the shared buffer.
    VULKAN_ENSURE(vkMapMemory(m_pipeline.m_device,
                              m_pipeline.m_sharedBufferMemory,
                              0,
                              sharedBufferSize,
                              0,
                              m_pipeline.m_bufferData));
}

//--------------------------------------------------------------
inline InteropVKHost::~InteropVKHost()
{
    vkUnmapMemory(m_pipeline.m_device,
                  m_pipeline.m_sharedBufferMemory);
}

} // namespace Vulkan
} // namespace Display
} // namespace Simple
