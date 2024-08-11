//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <cuda_runtime.h>

#include <assert.h>

//--------------------------------------------------------------
//! Controls whether CUDA debug should be performed and output.
//--------------------------------------------------------------
#define CUDA_DEBUG_NOTHING 0
#define CUDA_DEBUG_DEFAULT 1
#define CUDA_DEBUG_VERBOSE 2
#ifndef CUDA_DEBUG_SETTING
#   ifdef NDEBUG
#       define CUDA_DEBUG_SETTING CUDA_DEBUG_NOTHING
#   else
#       define CUDA_DEBUG_SETTING CUDA_DEBUG_DEFAULT
#   endif
#endif//CUDA_DEBUG_SETTING

//--------------------------------------------------------------
#if CUDA_DEBUG_SETTING
#define CUDA_ENSURE(cudaFunctionCall) do {                      \
    cudaError_t result = (cudaFunctionCall);                    \
    if (result != cudaSuccess) {                                \
        printf("CUDA Error: %s\n at %s:%d\n\n",                 \
               cudaGetErrorString(result), __FILE__, __LINE__); \
        assert(result == cudaSuccess);                          \
    }                                                           \
} while(0)
#else
#define CUDA_ENSURE(cudaFunctionCall) (cudaFunctionCall)
#endif // CUDA_DEBUG_SETTING
