//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <display/graphics/cuda/debug_cuda.h>
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
class InteropVKCuda : public InteropVK
{
public:
    InteropVKCuda(PipelineVK& a_pipeline);
    ~InteropVKCuda() override;

    InteropVKCuda(const InteropVKCuda&) = delete;
    InteropVKCuda& operator=(const InteropVKCuda&) = delete;

private:
    PipelineVK& m_pipeline;
    cudaExternalMemory_t m_cudaExternalMemory = nullptr;
};

//--------------------------------------------------------------
inline InteropVKCuda::InteropVKCuda(PipelineVK& a_pipeline)
    : m_pipeline(a_pipeline)
{
    // Describe the external memory buffer.
    VkExternalMemoryBufferCreateInfo externalMemoryBufferCreateInfo = {};
    externalMemoryBufferCreateInfo.handleTypes = m_pipeline.m_externalMemoryHandleType;
    externalMemoryBufferCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;

    // Describe the export memory allocation.
    VkExportMemoryAllocateInfo exportMemoryAllocateInfo = {};
    exportMemoryAllocateInfo.handleTypes = m_pipeline.m_externalMemoryHandleType;
    exportMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;

    // Create the shared buffer.
    const VkDeviceSize sharedBufferSize = Buffer::MinSizeBytes(m_pipeline.m_bufferConfig);
    const VkDeviceSize allocationSize = m_pipeline.CreateBuffer(m_pipeline.m_sharedBuffer,
                                                                m_pipeline.m_sharedBufferMemory,
                                                                sharedBufferSize,
                                                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                &externalMemoryBufferCreateInfo,
                                                                &exportMemoryAllocateInfo);

    // Describe the CUDA memory handle.
    cudaExternalMemoryHandleDesc externalMemoryHandleDesc;
    {
    #if defined(_WIN64)
        assert(m_pipeline.m_externalMemoryHandleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);

        // Describe the shared memory handle.
        VkMemoryGetWin32HandleInfoKHR getWin32HandleInfo = {};
        getWin32HandleInfo.memory = m_pipeline.m_sharedBufferMemory;
        getWin32HandleInfo.handleType = m_pipeline.m_externalMemoryHandleType;
        getWin32HandleInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;

        // Get the shared memory handle.
        HANDLE sharedHandleWin32;
        //vkGetMemoryWin32HandleKHR(m_pipeline.m_device, &getWin32HandleInfo, &sharedHandle);
        PFN_vkGetMemoryWin32HandleKHR fpGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(m_pipeline.m_device,
                                                                                                                     "vkGetMemoryWin32HandleKHR");
        assert(fpGetMemoryWin32HandleKHR);
        VULKAN_ENSURE(fpGetMemoryWin32HandleKHR(m_pipeline.m_device,
                                                &getWin32HandleInfo,
                                                &sharedHandleWin32));

        // Describe the CUDA memory handle.
        memset(&externalMemoryHandleDesc, 0, sizeof(externalMemoryHandleDesc));
        externalMemoryHandleDesc.handle.win32.handle = sharedHandleWin32;
        externalMemoryHandleDesc.size = allocationSize;
        externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
    #else
        assert(m_pipeline.m_externalMemoryHandleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);

        // Describe the shared memory handle.
        VkMemoryGetFdInfoKHR getFdInfo = {};
        getFdInfo.memory = m_pipeline.m_sharedBufferMemory;
        getFdInfo.handleType = m_pipeline.m_externalMemoryHandleType;
        getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;

        // Get the shared memory handle.
        int sharedHandleFd;
        //vkGetMemoryFdKHR(m_pipeline.m_device, &getFdInfo, &sharedHandleFd);
        PFN_vkGetMemoryFdKHR fpGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(m_pipeline.m_device,
                                                                                          "vkGetMemoryFdKHR");
        assert(fpGetMemoryFdKHR);
        VULKAN_ENSURE(fpGetMemoryFdKHR(m_pipeline.m_device,
                                       &getFdInfo,
                                       &sharedHandleFd));



        // Describe the CUDA memory handle.
        memset(&externalMemoryHandleDesc, 0, sizeof(externalMemoryHandleDesc));
        externalMemoryHandleDesc.handle.fd = sharedHandleFd;
        externalMemoryHandleDesc.size = allocationSize;
        externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
    #endif // defined(_WIN64)
    }

    // Import the CUDA memory handle.
    CUDA_ENSURE(cudaImportExternalMemory(&m_cudaExternalMemory,
                                         &externalMemoryHandleDesc));

    // Map the CUDA memory buffer.
    cudaExternalMemoryBufferDesc externalMemoryBufferDesc;
    memset(&externalMemoryBufferDesc, 0, sizeof(externalMemoryBufferDesc));
    externalMemoryBufferDesc.flags = 0;
    externalMemoryBufferDesc.offset = 0;
    externalMemoryBufferDesc.size = sharedBufferSize;
    CUDA_ENSURE(cudaExternalMemoryGetMappedBuffer(m_pipeline.m_bufferData,
                                                  m_cudaExternalMemory,
                                                  &externalMemoryBufferDesc));
}

//--------------------------------------------------------------
inline InteropVKCuda::~InteropVKCuda()
{
    // Free the mapped buffer data.
    CUDA_ENSURE(cudaFree(*m_pipeline.m_bufferData));

    // Destroy the external CUDA memory handle.
    CUDA_ENSURE(cudaDestroyExternalMemory(m_cudaExternalMemory));
}

} // namespace Vulkan
} // namespace Display
} // namespace Simple
