//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <display/window_implementation.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <assert.h>

//--------------------------------------------------------------
#if NDEBUG
#define X11_ENSURE(x11FunctionCall) (x11FunctionCall)
#else
#define X11_ENSURE(x11FunctionCall) do {                        \
    Status status = (x11FunctionCall);                          \
    assert(status);                                             \
} while(0)
#endif // NDEBUG

namespace Simple
{
namespace Display
{

//--------------------------------------------------------------
class WindowLinux : public Window::Implementation
{
public:
    WindowLinux(const Window::Config& a_config);
    ~WindowLinux() override;

    WindowLinux(const WindowLinux&) = delete;
    WindowLinux& operator=(const WindowLinux&) = delete;

protected:
    void Show() override;
    void Hide() override;
    void Close() override;

    void Maximize() override;
    void Minimize() override;
    void Restore() override;

    void FullScreenEnable() override;
    void FullScreenDisable() override;
    void FullScreenToggle() override;
    void FullScreen(Bool a_enable);

    void PumpWindowEventsOnce() override;
    void PumpWindowEventsUntilEmpty() override;

    bool IsFullScreen() const override;
    bool IsMaximized() const override;
    bool IsMinimized() const override;
    bool IsVisible() const override;
    bool IsClosed() const override;

    void GetDisplayDimensions(uint32_t& o_width,
                              uint32_t& o_height) const override;
    void GetWindowDimensions(uint32_t& o_width,
                             uint32_t& o_height) const override;

    void* GetNativeDisplayHandle() const override;
    void* GetNativeWindowHandle() const override;

    Window::NativeDeviceEvents* GetNativeDeviceEvents() override;
    Window::NativeInputEvents* GetNativeInputEvents() override;
    Window::NativeTextEvents* GetNativeTextEvents() override;

private:
    void ProcessEvent(const XEvent& a_event);
    void CacheFrameExtents();
    bool IsNativeWindowInState(const Atom& a_stateAtom) const;

    class ThreadLocalDisplay
    {
    public:
        ThreadLocalDisplay()
        {
            X11_ENSURE(XInitThreads());
            m_display = XOpenDisplay(nullptr);
            XLockDisplay(m_display);
        }
        ~ThreadLocalDisplay()
        {
            XUnlockDisplay(m_display);
            // ToDo: Causes Vulkan shutdown segfault.
            // XCloseDisplay(m_display);
        }

        ::Display* GetDisplay() const
        {
            return m_display;
        }
    private:
        ::Display* m_display = nullptr;
    };
    static thread_local ThreadLocalDisplay tl_display;

    Window::NativeInputEvents m_nativeInputEvents;
    ::Display* m_xDisplay = nullptr;
    ::Window m_xWindow = 0;
    bool m_isClosed = false;

    Atom m_xStateAtom;
    Atom m_xStateHiddenAtom;
    Atom m_xStateMaxHorzAtom;
    Atom m_xStateMaxVertAtom;
    Atom m_xStateFullScreenAtom;
    Atom m_xProtocolsAtom;
    Atom m_xDeleteWindowAtom;
    Atom m_xFrameExtentsAtom;

    uint32_t m_framePixelsLeft = 0;
    uint32_t m_framePixelsRight = 0;
    uint32_t m_framePixelsTop = 0;
    uint32_t m_framePixelsBottom = 0;
};

thread_local WindowLinux::ThreadLocalDisplay WindowLinux::tl_display;

//--------------------------------------------------------------
using ImplPtr = std::unique_ptr<Window::Implementation>;
ImplPtr Window::Implementation::Create(const Config& a_config)
{
    return std::make_unique<WindowLinux>(a_config);
}

//--------------------------------------------------------------
WindowLinux::WindowLinux(const Window::Config& a_config)
{
    // Store the thread local native display.
    m_xDisplay = tl_display.GetDisplay();

    // Create the native window.
    const uint32_t blackPixel = BlackPixel(m_xDisplay,
                                           DefaultScreen(m_xDisplay));
    XSetWindowAttributes windowAttributes;
    windowAttributes.background_pixmap = None;
    windowAttributes.background_pixel = blackPixel;
    windowAttributes.border_pixmap = None;
    windowAttributes.border_pixel = blackPixel;
    windowAttributes.backing_store = NotUseful;
    const unsigned long attributeMask = (CWBackPixmap |
                                         CWBackPixel |
                                         CWBorderPixmap |
                                         CWBorderPixel |
                                         CWBackingStore);
    m_xWindow = XCreateWindow(m_xDisplay,
                              DefaultRootWindow(m_xDisplay),
                              a_config.initialPositionX,
                              a_config.initialPositionY,
                              a_config.initialWidth,
                              a_config.initialHeight,
                              0,
                              CopyFromParent,
                              InputOutput,
                              CopyFromParent,
                              attributeMask,
                              &windowAttributes);
    assert(m_xWindow);

    // Set the name of the native window.
    X11_ENSURE(XStoreName(m_xDisplay,
                          m_xWindow,
                          a_config.titleUTF8.c_str()));

    // Define various atom ids that are needed.
    m_xStateAtom = XInternAtom(m_xDisplay, "_NET_WM_STATE", False);
    m_xStateHiddenAtom = XInternAtom(m_xDisplay, "_NET_WM_STATE_HIDDEN", False);
    m_xStateMaxHorzAtom = XInternAtom(m_xDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    m_xStateMaxVertAtom = XInternAtom(m_xDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    m_xStateFullScreenAtom = XInternAtom(m_xDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    m_xProtocolsAtom = XInternAtom(m_xDisplay, "WM_PROTOCOLS", False);
    m_xDeleteWindowAtom = XInternAtom(m_xDisplay, "WM_DELETE_WINDOW", False);
    m_xFrameExtentsAtom = XInternAtom(m_xDisplay, "_NET_FRAME_EXTENTS", False);

    // Prevent the window manager from deleting the window.
    X11_ENSURE(XSetWMProtocols(m_xDisplay,
                               m_xWindow,
                               &m_xDeleteWindowAtom,
                               1));

    // Select the events to process.
    const long windowEventMask = ExposureMask |
                                 VisibilityChangeMask |
                                 StructureNotifyMask |
                                 SubstructureNotifyMask |
                                 SubstructureRedirectMask |
                                 FocusChangeMask |
                                 PropertyChangeMask;
    X11_ENSURE(XSelectInput(m_xDisplay,
                            m_xWindow,
                            windowEventMask));
    const long rootWindowEventMask = StructureNotifyMask |
                                     SubstructureNotifyMask |
                                     PropertyChangeMask;
    X11_ENSURE(XSelectInput(m_xDisplay,
                            DefaultRootWindow(m_xDisplay),
                            rootWindowEventMask));

    // Flush the native window creation.
    X11_ENSURE(XFlush(m_xDisplay));
}

//--------------------------------------------------------------
WindowLinux::~WindowLinux()
{
    // Hide the native window.
    Hide();

    // Destroy the native window.
    X11_ENSURE(XDestroyWindow(m_xDisplay, m_xWindow));

    // Flush the native window destruction.
    X11_ENSURE(XFlush(m_xDisplay));
}

//--------------------------------------------------------------
void WindowLinux::Show()
{
    if (IsVisible() || m_isClosed)
    {
        return;
    }

    // Show the native window.
    X11_ENSURE(XClearWindow(m_xDisplay, m_xWindow));
    X11_ENSURE(XMapRaised(m_xDisplay, m_xWindow));
    X11_ENSURE(XFlush(m_xDisplay));

    // Wait for the window to become visible.
    while (!IsVisible()) {}
}

//--------------------------------------------------------------
void WindowLinux::Hide()
{
    if (!IsVisible() || m_isClosed)
    {
        return;
    }

    // Restore the native window.
    Restore();

    // Hide the native window.
    X11_ENSURE(XUnmapWindow(m_xDisplay, m_xWindow));
    X11_ENSURE(XFlush(m_xDisplay));

    // Wait for the window to become invisible.
    while (IsVisible()) {}
}

//--------------------------------------------------------------
void WindowLinux::Close()
{
    if (m_isClosed)
    {
        return;
    }

    // Hide the native window and set as closed.
    Hide();
    m_isClosed = true;
}

//--------------------------------------------------------------
void WindowLinux::Maximize()
{
    if (IsMaximized() || !IsVisible() || m_isClosed)
    {
        return;
    }

    // Maximize the native window.
    XEvent maximizeEvent;
    maximizeEvent.type = ClientMessage;
    maximizeEvent.xclient.window = m_xWindow;
    maximizeEvent.xclient.message_type = m_xStateAtom;
    maximizeEvent.xclient.format = 32;
    maximizeEvent.xclient.data.l[0] = 1; // Add property
    maximizeEvent.xclient.data.l[1] = m_xStateMaxHorzAtom;
    maximizeEvent.xclient.data.l[2] = m_xStateMaxVertAtom;
    X11_ENSURE(XSendEvent(m_xDisplay,
                          DefaultRootWindow(m_xDisplay),
                          False,
                          SubstructureNotifyMask | SubstructureRedirectMask,
                          &maximizeEvent));
    X11_ENSURE(XFlush(m_xDisplay));

    // Wait for the native window to maximize.
    while (!IsMaximized()) {}
}

//--------------------------------------------------------------
void WindowLinux::Minimize()
{
    if (IsMinimized() || !IsVisible() || m_isClosed)
    {
        return;
    }

    // Minimize the native window.
    X11_ENSURE(XIconifyWindow(m_xDisplay,
                              m_xWindow,
                              DefaultScreen(m_xDisplay)));
    X11_ENSURE(XFlush(m_xDisplay));

    // Wait for the native window to minimize.
    // ToDo: IsMinimized doesn't work.
    // while (!IsMinimized()) {}
}

//--------------------------------------------------------------
void WindowLinux::Restore()
{
    if (!IsVisible() || m_isClosed)
    {
        return;
    }

    if (IsFullScreen())
    {
        FullScreenDisable();
    }

    if (IsMaximized())
    {
        // Restore the native window.
        XEvent restoreEvent;
        restoreEvent.type = ClientMessage;
        restoreEvent.xclient.window = m_xWindow;
        restoreEvent.xclient.message_type = m_xStateAtom;
        restoreEvent.xclient.format = 32;
        restoreEvent.xclient.data.l[0] = 0; // Remove property
        restoreEvent.xclient.data.l[1] = m_xStateMaxHorzAtom;
        restoreEvent.xclient.data.l[2] = m_xStateMaxVertAtom;
        X11_ENSURE(XSendEvent(m_xDisplay,
                              DefaultRootWindow(m_xDisplay),
                              False,
                              SubstructureNotifyMask | SubstructureRedirectMask,
                              &restoreEvent));
        X11_ENSURE(XFlush(m_xDisplay));

        // Wait for the native window to restore.
        while (IsMaximized()) {}
    }

    if (IsMinimized())
    {
        // Restore the native window.
        // ToDo: How to restore from iconified/minimized?

        // Wait for the native window to restore.
        // ToDo: IsMinimized doesn't work.
        // while (IsMinimized()) {}
    }
}

//--------------------------------------------------------------
void WindowLinux::FullScreenEnable()
{
    if (!IsFullScreen())
    {
        Maximize();
        FullScreen(True);
    }
}

//--------------------------------------------------------------
void WindowLinux::FullScreenDisable()
{
    if (IsFullScreen())
    {
        FullScreen(False);
    }
}

//--------------------------------------------------------------
void WindowLinux::FullScreenToggle()
{
    FullScreen(!IsFullScreen());
}

//--------------------------------------------------------------
void WindowLinux::FullScreen(Bool a_enable)
{
    if (!IsVisible() || m_isClosed)
    {
        return;
    }

    XEvent fulScreenEvent;
    fulScreenEvent.type = ClientMessage;
    fulScreenEvent.xclient.window = m_xWindow;
    fulScreenEvent.xclient.message_type = m_xStateAtom;
    fulScreenEvent.xclient.format = 32;
    fulScreenEvent.xclient.data.l[0] = a_enable;
    fulScreenEvent.xclient.data.l[1] = m_xStateFullScreenAtom;
    fulScreenEvent.xclient.data.l[2] = None;
    X11_ENSURE(XSendEvent(m_xDisplay,
                          DefaultRootWindow(m_xDisplay),
                          False,
                          SubstructureNotifyMask | SubstructureRedirectMask,
                          &fulScreenEvent));
    X11_ENSURE(XFlush(m_xDisplay));

    // Wait for the window to transition to/from full screen.
    while (a_enable != IsFullScreen()) {}
}

//--------------------------------------------------------------
static Bool ShouldProcessEvent(::Display*, XEvent*, XPointer)
{
    return true;
}

//--------------------------------------------------------------
void WindowLinux::PumpWindowEventsOnce()
{
    XEvent xEvent;
    if (XCheckIfEvent(m_xDisplay,
                      &xEvent,
                      ShouldProcessEvent,
                      (XPointer)&m_xWindow))
    {
        ProcessEvent(xEvent);
    }
}

//--------------------------------------------------------------
void WindowLinux::PumpWindowEventsUntilEmpty()
{
    XEvent xEvent;
    while (XCheckIfEvent(m_xDisplay,
                         &xEvent,
                         ShouldProcessEvent,
                         (XPointer)&m_xWindow))
    {
        ProcessEvent(xEvent);
    }
}

//--------------------------------------------------------------
bool WindowLinux::IsFullScreen() const
{
    return IsNativeWindowInState(m_xStateFullScreenAtom);
}

//--------------------------------------------------------------
bool WindowLinux::IsMaximized() const
{
    return IsNativeWindowInState(m_xStateMaxHorzAtom) &&
           IsNativeWindowInState(m_xStateMaxVertAtom);
}

//--------------------------------------------------------------
bool WindowLinux::IsMinimized() const
{
    return IsNativeWindowInState(m_xStateHiddenAtom);
}

//--------------------------------------------------------------
bool WindowLinux::IsVisible() const
{
    XWindowAttributes windowAttributes;
    if (XGetWindowAttributes(m_xDisplay,
                             m_xWindow,
                             &windowAttributes))
    {
        return windowAttributes.map_state == IsViewable;
    }
    return false;
}

//--------------------------------------------------------------
bool WindowLinux::IsClosed() const
{
    return m_isClosed;
}

//--------------------------------------------------------------
void WindowLinux::GetDisplayDimensions(uint32_t& o_width,
                                       uint32_t& o_height) const
{
    o_width = 0;
    o_height = 0;

    XWindowAttributes windowAttributes;
    if (XGetWindowAttributes(m_xDisplay,
                             m_xWindow,
                             &windowAttributes))
    {
        o_width = windowAttributes.width;
        o_height = windowAttributes.height;
    }
}

//--------------------------------------------------------------
void WindowLinux::GetWindowDimensions(uint32_t& o_width,
                                      uint32_t& o_height) const
{
    GetDisplayDimensions(o_width, o_height);
    o_width += m_framePixelsLeft + m_framePixelsRight;
    o_height += m_framePixelsTop + m_framePixelsBottom;
}

//--------------------------------------------------------------
void* WindowLinux::GetNativeDisplayHandle() const
{
    return m_xDisplay;
}

//--------------------------------------------------------------
void* WindowLinux::GetNativeWindowHandle() const
{
    return const_cast<::Window*>(&m_xWindow);
}

//--------------------------------------------------------------
Window::NativeDeviceEvents* WindowLinux::GetNativeDeviceEvents()
{
    return nullptr;
}

//--------------------------------------------------------------
Window::NativeInputEvents* WindowLinux::GetNativeInputEvents()
{
    return &m_nativeInputEvents;
}

//--------------------------------------------------------------
Window::NativeTextEvents* WindowLinux::GetNativeTextEvents()
{
    return nullptr;
}

//--------------------------------------------------------------
void WindowLinux::ProcessEvent(const XEvent& a_event)
{
    if (a_event.type == ClientMessage &&
        a_event.xclient.message_type == m_xProtocolsAtom &&
        (Atom)a_event.xclient.data.l[0] == m_xDeleteWindowAtom)
    {
        Close();
    }
    else if (a_event.type == PropertyNotify &&
             a_event.xproperty.state == PropertyNewValue &&
             a_event.xproperty.atom == m_xFrameExtentsAtom)
    {
        CacheFrameExtents();
    }
    else
    {
        m_nativeInputEvents.Dispatch(&a_event);
    }
}

//--------------------------------------------------------------
void WindowLinux::CacheFrameExtents()
{
    Atom actualType;
    int actualFormat = 0;
    unsigned long bytesAfter = 0;
    unsigned long numProperties = 0;
    unsigned char* propertyData = nullptr;
    if (XGetWindowProperty(m_xDisplay,
                           m_xWindow,
                           m_xFrameExtentsAtom,
                           0,
                           4,
                           False,
                           XA_CARDINAL,
                           &actualType,
                           &actualFormat,
                           &numProperties,
                           &bytesAfter,
                           &propertyData) == Success)
    {
        long* frameExtents = (long*)propertyData;
        const long extentsLeft = frameExtents[0];
        const long extentsRight = frameExtents[1];
        const long extentsTop = frameExtents[2];
        const long extentsBottom = frameExtents[3];
        assert(extentsLeft >= 0);
        assert(extentsRight >= 0);
        assert(extentsTop >= 0);
        assert(extentsBottom >= 0);
        m_framePixelsLeft = extentsLeft;
        m_framePixelsRight = extentsRight;
        m_framePixelsTop = extentsTop;
        m_framePixelsBottom = extentsBottom;
    }
    XFree(propertyData);
}

//--------------------------------------------------------------
bool WindowLinux::IsNativeWindowInState(const Atom& a_stateAtom) const
{
    Atom actualType;
    int actualFormat = 0;
    unsigned long bytesAfter = 0;
    unsigned long numProperties = 0;
    unsigned char* properties;
    if (XGetWindowProperty(m_xDisplay,
                           m_xWindow,
                           m_xStateAtom,
                           0,
                           1024,
                           False,
                           XA_ATOM,
                           &actualType,
                           &actualFormat,
                           &numProperties,
                           &bytesAfter,
                           &properties) == Success)
    {
        Atom* atomProperties = (Atom*)properties;
        for (unsigned long i = 0; i < numProperties; ++i)
        {
            if (atomProperties[i] == a_stateAtom)
            {
                return true;
            }
        }
    }
    XFree(properties);
    return false;
}

} // Display
} // Simple
