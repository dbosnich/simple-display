//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#define NOMINMAX
#include <windows.h>

#include <assert.h>

//--------------------------------------------------------------
//! Controls whether D3D12 debug should be performed and output.
//--------------------------------------------------------------
#define D3D12_DEBUG_NOTHING 0
#define D3D12_DEBUG_DEFAULT 1
#define D3D12_DEBUG_VERBOSE 2
#ifndef D3D12_DEBUG_SETTING
#   ifdef NDEBUG
#       define D3D12_DEBUG_SETTING D3D12_DEBUG_NOTHING
#   else
#       define D3D12_DEBUG_SETTING D3D12_DEBUG_DEFAULT
#   endif
#endif//D3D12_DEBUG_SETTING

//--------------------------------------------------------------
void PrintError(HRESULT a_result, const char* a_file, int a_line)
{
    LPVOID errorMessage;
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS;

    ::FormatMessageA(flags,
                     NULL,
                     a_result,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPTSTR)&errorMessage,
                     0,
                     NULL);

    printf("D3D12 Error: %s at %s:%d\n\n",
           (char*)errorMessage, a_file, a_line);
    ::LocalFree(errorMessage);
}

//--------------------------------------------------------------
#if D3D12_DEBUG_SETTING
#define D3D12_ENSURE(d3d12FunctionCall) do {                    \
    HRESULT result = (d3d12FunctionCall);                       \
    if (FAILED(result)) {                                       \
        PrintError(result, __FILE__, __LINE__);                 \
        assert(SUCCEEDED(result));                              \
    }                                                           \
} while(0)
#else
#define D3D12_ENSURE(d3d12FunctionCall) (d3d12FunctionCall)
#endif // D3D12_DEBUG_SETTING
