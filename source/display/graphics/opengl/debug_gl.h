//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <assert.h>

//--------------------------------------------------------------
//! Controls whether OpenGL debug should be performed and output.
//--------------------------------------------------------------
#define OPENGL_DEBUG_NOTHING 0
#define OPENGL_DEBUG_DEFAULT 1
#define OPENGL_DEBUG_VERBOSE 2
#ifndef OPENGL_DEBUG_SETTING
#   ifdef NDEBUG
#       define OPENGL_DEBUG_SETTING OPENGL_DEBUG_NOTHING
#   else
#       define OPENGL_DEBUG_SETTING OPENGL_DEBUG_DEFAULT
#   endif
#endif//OPENGL_DEBUG_SETTING
