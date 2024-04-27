//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <memory>
#include <string>

//! @file

//--------------------------------------------------------------
//! The default title (utf8) used to create any display window.
//--------------------------------------------------------------
#ifndef DEFAULT_WINDOW_TITLE
#define DEFAULT_WINDOW_TITLE "Simple Display Window"
#endif//DEFAULT_WINDOW_TITLE

//--------------------------------------------------------------
//! The default initial width of any window, measured in pixels.
//--------------------------------------------------------------
#ifndef DEFAULT_WINDOW_WIDTH
#define DEFAULT_WINDOW_WIDTH 800
#endif//DEFAULT_WINDOW_WIDTH

//--------------------------------------------------------------
//! The default initial height of any window, measured in pixels.
//--------------------------------------------------------------
#ifndef DEFAULT_WINDOW_HEIGHT
#define DEFAULT_WINDOW_HEIGHT 600
#endif//DEFAULT_WINDOW_HEIGHT

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{

//--------------------------------------------------------------
//! Class that represents a system window which can be displayed.
//!
//! The Simple::Display::Window class encapsulates functionality
//! for creating, resizing, and updating a native system window,
//! which doubles as a display device for all desktop platforms.
//--------------------------------------------------------------
class Window
{
public:
    class Implementation;

    //----------------------------------------------------------
    //! Values needed to define Simple::Display::Window objects.
    //----------------------------------------------------------
    struct Config
    {
        //! The title (utf8) used to create the display window.
        std::string titleUTF8 = DEFAULT_WINDOW_TITLE;

        //! The initial width of the window, measured in pixels.
        uint32_t initialWidth = DEFAULT_WINDOW_WIDTH;

        //! The initial height of the window, measured in pixels.
        uint32_t initialHeight = DEFAULT_WINDOW_HEIGHT;
    };

    Window(const Config& a_config);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void Show();
    void Hide();
    void Close();

    void Maximize();
    void Minimize();

    void FullScreenEnable();
    void FullScreenDisable();
    void FullScreenToggle();

    void PumpWindowEventsOnce();
    void PumpWindowEventsUntilEmpty();

    bool IsFullScreen() const;
    bool IsMinimized() const;
    bool IsMaximized() const;
    bool IsVisible() const;
    bool IsClosed() const;

    void GetDisplayDimensions(uint32_t& o_displayWidth,
                              uint32_t& o_displayHeight) const;
    void GetWindowDimensions(uint32_t& o_windowWidth,
                             uint32_t& o_windowHeight) const;

    void* GetNativeDisplayHandle() const;
    void* GetNativeWindowHandle() const;

private:
    const std::unique_ptr<Implementation> m_pimpl;
};

} // namespace Display
} // namespace Simple
