//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

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
//! The default initial x position of the window, set in pixels.
//--------------------------------------------------------------
#ifndef DEFAULT_WINDOW_POSITION_X
#define DEFAULT_WINDOW_POSITION_X 0
#endif//DEFAULT_WINDOW_POSITION_X

//--------------------------------------------------------------
//! The default initial y position of the window, set in pixels.
//--------------------------------------------------------------
#ifndef DEFAULT_WINDOW_POSITION_Y
#define DEFAULT_WINDOW_POSITION_Y 0
#endif//DEFAULT_WINDOW_POSITION_Y

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

        //! The initial x position of the window, set in pixels.
        uint32_t initialPositionX = DEFAULT_WINDOW_POSITION_X;

        //! The initial y position of the window, set in pixels.
        uint32_t initialPositionY = DEFAULT_WINDOW_POSITION_Y;
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
    void Restore();

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

    //----------------------------------------------------------
    //! Class that manages a collection of native event listener
    //! functions to be invoked each time an event is dispatched.
    //!
    //! \tparam T The type of the native event to be dispatched.
    //----------------------------------------------------------
    template<typename T>
    class NativeEvents
    {
    public:
        using Callable = std::function<void(T)>;
        using Listener = std::shared_ptr<Callable>;

        [[nodiscard]]
        Listener Register(const Callable& a_callable);
        bool Remove(const Listener& a_listener);
        void Dispatch(T a_nativeEvent);

    private:
        std::vector<std::weak_ptr<Callable>> m_listeners;
        std::mutex m_listenersMutex;
    };

    using NativeInputEvents = NativeEvents<const void*>;
    using NativeTextEvents = NativeEvents<const std::string&>;

    NativeInputEvents* GetNativeInputEvents() const;
    NativeTextEvents* GetNativeTextEvents() const;

private:
    const std::unique_ptr<Implementation> m_pimpl;
};

} // namespace Display
} // namespace Simple
