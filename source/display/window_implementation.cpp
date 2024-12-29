//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <display/window_implementation.h>

using namespace Simple::Display;

//--------------------------------------------------------------
//! Create a display window using supplied configuration values.
//!
//! \param[in] a_config The values needed to create the window.
//--------------------------------------------------------------
Window::Window(const Config& a_config)
    : m_pimpl(Window::Implementation::Create(a_config))
{
}

//--------------------------------------------------------------
Window::~Window()
{
}

//--------------------------------------------------------------
//! Show the window so that it is visible on the system display.
//--------------------------------------------------------------
void Window::Show()
{
    if (m_pimpl)
    {
        m_pimpl->Show();
    }
}

//--------------------------------------------------------------
//! Hide the window so it is not visible on the system display.
//--------------------------------------------------------------
void Window::Hide()
{
    if (m_pimpl)
    {
        m_pimpl->Hide();
    }
}

//--------------------------------------------------------------
//! Close the window, analogous to doing so from the X UI button.
//--------------------------------------------------------------
void Window::Close()
{
    if (m_pimpl)
    {
        m_pimpl->Close();
    }
}

//--------------------------------------------------------------
//! Maximize the window so the window itself fills the display.
//--------------------------------------------------------------
void Window::Maximize()
{
    if (m_pimpl)
    {
        m_pimpl->Maximize();
    }
}

//--------------------------------------------------------------
//! Minimize the window so that it collapses to the system tray.
//--------------------------------------------------------------
void Window::Minimize()
{
    if (m_pimpl)
    {
        m_pimpl->Minimize();
    }
}

//--------------------------------------------------------------
//! Restore the window from maximized/minimzed/full screen state.
//--------------------------------------------------------------
void Window::Restore()
{
    if (m_pimpl)
    {
        m_pimpl->Restore();
    }
}

//--------------------------------------------------------------
//! Enable full screen so the window contents fill the display.
//--------------------------------------------------------------
void Window::FullScreenEnable()
{
    if (m_pimpl)
    {
        m_pimpl->FullScreenEnable();
    }
}

//--------------------------------------------------------------
//! Disable full screen to restore the prior state of the window.
//--------------------------------------------------------------
void Window::FullScreenDisable()
{
    if (m_pimpl)
    {
        m_pimpl->FullScreenDisable();
    }
}

//--------------------------------------------------------------
//! Toggle the window full screen state between enabled/disabled.
//--------------------------------------------------------------
void Window::FullScreenToggle()
{
    if (m_pimpl)
    {
        m_pimpl->FullScreenToggle();
    }
}

//--------------------------------------------------------------
//! Process one pending system event associated with the window.
//--------------------------------------------------------------
void Window::PumpWindowEventsOnce()
{
    if (m_pimpl)
    {
        m_pimpl->PumpWindowEventsOnce();
    }
}

//--------------------------------------------------------------
//! Process all pending system events associated with the window.
//--------------------------------------------------------------
void Window::PumpWindowEventsUntilEmpty()
{
    if (m_pimpl)
    {
        m_pimpl->PumpWindowEventsUntilEmpty();
    }
}

//--------------------------------------------------------------
//! Query whether the window is currently in a full screen state.
//!
//! \return True if the window is full screen, false otherwise.
//--------------------------------------------------------------
bool Window::IsFullScreen() const
{
    return m_pimpl ? m_pimpl->IsFullScreen() : false;
}

//--------------------------------------------------------------
//! Query whether the window is currently in the maximized state.
//!
//! \return True if the window is now maximized, false otherwise.
//--------------------------------------------------------------
bool Window::IsMaximized() const
{
    return m_pimpl ? m_pimpl->IsMaximized() : false;
}

//--------------------------------------------------------------
//! Query whether the window is currently in the minimized state.
//!
//! \return True if the window is now minimized, false otherwise.
//--------------------------------------------------------------
bool Window::IsMinimized() const
{
    return m_pimpl ? m_pimpl->IsMinimized() : false;
}

//--------------------------------------------------------------
//! Query whether the window is currently visible on the display.
//!
//! \return True if the window is now visible, false otherwise.
//--------------------------------------------------------------
bool Window::IsVisible() const
{
    return m_pimpl ? m_pimpl->IsVisible() : false;
}

//--------------------------------------------------------------
//! Query whether the window has been closed by user or program.
//!
//! \return True if the window is now closed, false otherwise.
//--------------------------------------------------------------
bool Window::IsClosed() const
{
    return m_pimpl ? m_pimpl->IsClosed() : false;
}

//--------------------------------------------------------------
//! Get the current dimensions of the content being displayed.
//!
//! \param[out] o_width The pixel width of the window content.
//! \param[out] o_height The pixel height of the window content.
//--------------------------------------------------------------
void Window::GetDisplayDimensions(uint32_t& o_width,
                                  uint32_t& o_height) const
{
    if (m_pimpl)
    {
        m_pimpl->GetDisplayDimensions(o_width, o_height);
    }
    else
    {
        o_width = 0;
        o_height = 0;
    }
}

//--------------------------------------------------------------
//! Get the current dimensions of the window including title bar.
//!
//! \param[out] o_width The pixel width of the entire window.
//! \param[out] o_height The pixel height of the entire window.
//--------------------------------------------------------------
void Window::GetWindowDimensions(uint32_t& o_width,
                                 uint32_t& o_height) const
{
    if (m_pimpl)
    {
        m_pimpl->GetWindowDimensions(o_width, o_height);
    }
    else
    {
        o_width = 0;
        o_height = 0;
    }
}

//--------------------------------------------------------------
//! Get a pointer/handle to the platform specific native display.
//!
//! \return Native display handle if it exists nullptr otherwise.
//--------------------------------------------------------------
void* Window::GetNativeDisplayHandle() const
{
    return m_pimpl ? m_pimpl->GetNativeDisplayHandle() : nullptr;
}

//--------------------------------------------------------------
//! Get a pointer/handle to the platform specific native window.
//!
//! \return Native window handle if it exists, nullptr otherwise.
//--------------------------------------------------------------
void* Window::GetNativeWindowHandle() const
{
    return m_pimpl ? m_pimpl->GetNativeWindowHandle() : nullptr;
}

//--------------------------------------------------------------
//! Get a pointer to the platform specific native input events.
//!
//! \return Native input events if they exist, nullptr otherwise.
//--------------------------------------------------------------
Window::NativeInputEvents* Window::GetNativeInputEvents() const
{
    return m_pimpl ? m_pimpl->GetNativeInputEvents() : nullptr;
}

//--------------------------------------------------------------
//! Get a pointer to the platform specific native text events.
//!
//! \return Native text events if they exist, nullptr otherwise.
//--------------------------------------------------------------
Window::NativeTextEvents* Window::GetNativeTextEvents() const
{
    return m_pimpl ? m_pimpl->GetNativeTextEvents() : nullptr;
}

//--------------------------------------------------------------
//! Registers a callable to invoke when each event is dispatched.
//!
//! \param[in] a_callable A callable object that will be invoked.
//! \return Listener to retain while callable should be invoked.
//!         Release all references to 'deregister' the callable.
//--------------------------------------------------------------
template<typename T>
typename Window::NativeEvents<T>::Listener
Window::NativeEvents<T>::Register(const Callable& a_callable)
{
    // Create the listener and add it to the container.
    std::lock_guard<std::mutex> lock(m_listenersMutex);
    Listener listener = std::make_shared<Callable>(a_callable);
    m_listeners.push_back(listener);
    return listener;
}

//--------------------------------------------------------------
//! Remove a listener so not invoked when events are dispatched.
//!
//! \param[in] a_listener Listener object to stop being invoked.
//! \return True if the listener was removed or false otherwise.
//--------------------------------------------------------------
template<typename T>
bool Window::NativeEvents<T>::Remove(const Listener& a_listener)
{
    // Find and remove the listener from the container.
    std::lock_guard<std::mutex> lock(m_listenersMutex);
    const auto& listenersBegin = m_listeners.begin();
    const auto& listenersEnd = m_listeners.end();
    for (auto it = listenersBegin; it != listenersEnd; ++it)
    {
        if (it->lock() == a_listener)
        {
            m_listeners.erase(it);
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------
template<typename T>
void Window::NativeEvents<T>::Dispatch(T a_nativeEvent)
{
    // Gather non-expired listeners.
    std::vector<Listener> listeners;
    {
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        listeners.reserve(m_listeners.size());

        // Iterate over all listeners.
        auto it = m_listeners.begin();
        while (it != m_listeners.end())
        {
            if (Listener listener = it->lock())
            {
                // Copy non-expired listeners.
                listeners.push_back(listener);
                ++it;
            }
            else
            {
                // Prune expired listeners.
                it = m_listeners.erase(it);
            }
        }
    }

    // Send the event to each listener.
    for (Listener listener : listeners)
    {
        if (Callable callable = *listener)
        {
            callable(a_nativeEvent);
        }
    }
}

template class Window::NativeEvents<const void*>;
template class Window::NativeEvents<const std::string&>;
