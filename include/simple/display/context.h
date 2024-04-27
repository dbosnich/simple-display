//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include "buffer.h"
#include "window.h"

//! @file

//--------------------------------------------------------------
//! The default graphics API used to create any display context.
//--------------------------------------------------------------
#ifndef DEFAULT_GRAPHICS_API
#define DEFAULT_GRAPHICS_API GraphicsAPI::NATIVE
#endif//DEFAULT_GRAPHICS_API

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{

//--------------------------------------------------------------
//! Class that represents a graphics API used to create buffers.
//!
//! The Simple::Display::Context class provides functionality to
//! present a pixel buffer to a display device/window each frame,
//! acting as a link between the native system and graphics API.
//--------------------------------------------------------------
class Context
{
public:
    class Implementation;

    //----------------------------------------------------------
    //! The graphics API to use for creating the display buffer.
    //----------------------------------------------------------
    enum class GraphicsAPI
    {
        NONE = 0,   //!< None/unknown/invalid graphics API.
        NATIVE,     //!< The system native graphics API.
        OPENGL,     //!< The OpenGL graphics API.
        VULKAN      //!< The Vulkan graphics API.
    };

    //----------------------------------------------------------
    //! Values needed to define Simple::Display::Context objects.
    //----------------------------------------------------------
    struct Config
    {
        //! The configuration used to create the display buffer.
        Buffer::Config bufferConfig;

        //! The configuration used to create the display window.
        Window::Config windowConfig;

        //! The graphics API used to create the display context.
        GraphicsAPI graphicsAPI = DEFAULT_GRAPHICS_API;
    };

    Context(const Config& a_config);
    ~Context();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Buffer& GetBuffer() const;
    Window* GetWindow() const;

    void OnFrameStart();
    void OnFrameEnded();

private:
    const std::unique_ptr<Implementation> m_pimpl;
};

} // namespace Display
} // namespace Simple
