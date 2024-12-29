//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <simple/display/window.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{

//--------------------------------------------------------------
class Window::Implementation
{
public:
    // Public destructor for unique_ptr.
    virtual ~Implementation() = default;

protected:
    // Platform specific factory function creates implementation.
    static std::unique_ptr<Implementation> Create(const Config&);

    friend class Window;
    Implementation() = default;

    Implementation(const Implementation&) = delete;
    Implementation& operator=(const Implementation&) = delete;

    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Close() = 0;

    virtual void Maximize() = 0;
    virtual void Minimize() = 0;
    virtual void Restore() = 0;

    virtual void FullScreenEnable() = 0;
    virtual void FullScreenDisable() = 0;
    virtual void FullScreenToggle() = 0;

    virtual void PumpWindowEventsOnce() = 0;
    virtual void PumpWindowEventsUntilEmpty() = 0;

    virtual bool IsFullScreen() const = 0;
    virtual bool IsMaximized() const = 0;
    virtual bool IsMinimized() const = 0;
    virtual bool IsVisible() const = 0;
    virtual bool IsClosed() const = 0;

    virtual void GetDisplayDimensions(uint32_t&,
                                      uint32_t&) const = 0;
    virtual void GetWindowDimensions(uint32_t&,
                                     uint32_t&) const = 0;

    virtual void* GetNativeDisplayHandle() const = 0;
    virtual void* GetNativeWindowHandle() const = 0;

    virtual NativeInputEvents* GetNativeInputEvents() = 0;
    virtual NativeTextEvents* GetNativeTextEvents() = 0;
};

} // namespace Display
} // namespace Simple
