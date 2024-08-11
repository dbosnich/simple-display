//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <vulkan/vulkan.h>

#include <assert.h>

//--------------------------------------------------------------
//! Controls whether Vulkan debug should be performed and output.
//--------------------------------------------------------------
#define VULKAN_DEBUG_NOTHING 0
#define VULKAN_DEBUG_DEFAULT 1
#define VULKAN_DEBUG_VERBOSE 2
#ifndef VULKAN_DEBUG_SETTING
#   ifdef NDEBUG
#       define VULKAN_DEBUG_SETTING VULKAN_DEBUG_NOTHING
#   else
#       define VULKAN_DEBUG_SETTING VULKAN_DEBUG_DEFAULT
#   endif
#endif//VULKAN_DEBUG_SETTING

//--------------------------------------------------------------
#if VULKAN_DEBUG_SETTING
#define VULKAN_ENSURE(vulkanFunctionCall) do {                  \
    VkResult result = (vulkanFunctionCall);                     \
    assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);\
} while(0)
#else
#define VULKAN_ENSURE(vulkanFunctionCall) (vulkanFunctionCall)
#endif // VULKAN_DEBUG_SETTING
