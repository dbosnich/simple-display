//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <display/window_implementation.h>

#import <Cocoa/Cocoa.h>

using namespace Simple::Display;

//--------------------------------------------------------------
class WindowMacOS : public Window::Implementation
{
public:
    WindowMacOS(const Window::Config& a_config);
    ~WindowMacOS() override;

    WindowMacOS(const WindowMacOS&) = delete;
    WindowMacOS& operator=(const WindowMacOS&) = delete;

    void OnNativeWindowDidEnterFullScreen();
    void OnNativeWindowDidExitFullScreen();
    void OnNativeWindowWillClose();

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

    Window::NativeInputEvents* GetNativeInputEvents() override;
    Window::NativeTextEvents* GetNativeTextEvents() override;

private:
    Window::NativeInputEvents m_nativeInputEvents;
    NSWindow* m_nsWindow;
    bool m_isTransitioning = false;
    bool m_isVisible = false;
    bool m_isClosed = false;
};

//--------------------------------------------------------------
using ImplPtr = std::unique_ptr<Window::Implementation>;
ImplPtr Window::Implementation::Create(const Config& a_config)
{
    return std::make_unique<WindowMacOS>(a_config);
}

//--------------------------------------------------------------
@interface WindowDelegate : NSObject<NSWindowDelegate>
-(instancetype)initWithWindow : (WindowMacOS*) a_window;
@end

//--------------------------------------------------------------
WindowMacOS::WindowMacOS(const Window::Config& a_config)
    : m_nsWindow(nullptr)
{
    @autoreleasepool
    {
        // Initialise the display environment.
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];

        // Setup the window.
        NSRect windowRect = NSMakeRect(a_config.initialPositionX,
                                       a_config.initialPositionY,
                                       a_config.initialWidth,
                                       a_config.initialHeight);
        NSWindowStyleMask windowStyle = NSWindowStyleMaskTitled |
                                        NSWindowStyleMaskClosable |
                                        NSWindowStyleMaskResizable |
                                        NSWindowStyleMaskMiniaturizable;
        NSRect contentRect = [NSWindow contentRectForFrameRect: windowRect
                                                     styleMask: windowStyle];

        // Create the window.
        m_nsWindow = [NSWindow alloc];
        [m_nsWindow initWithContentRect: contentRect
                              styleMask: windowStyle
                                backing: NSBackingStoreBuffered
                                  defer: NO];

        // Set the window title and allow it to enter full screen.
        m_nsWindow.title = [NSString stringWithUTF8String: a_config.titleUTF8.c_str()];
        [m_nsWindow setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];

        // Create and set the delegate for handling window events.
        WindowDelegate* windowDelegate = [[WindowDelegate alloc] initWithWindow: this];
        [m_nsWindow setDelegate: windowDelegate];
    }
}

//--------------------------------------------------------------
WindowMacOS::~WindowMacOS()
{
    // Close the native window.
    Close();

    // Release the native window.
    [m_nsWindow release];
}

//--------------------------------------------------------------
void WindowMacOS::Show()
{
    if (m_isVisible || m_isClosed)
    {
        return;
    }

    // Show the native window.
    [m_nsWindow makeKeyWindow];
    [m_nsWindow orderFrontRegardless];
    [m_nsWindow makeMainWindow];
    m_isVisible = true;
}

//--------------------------------------------------------------
void WindowMacOS::Hide()
{
    if (!m_isVisible || m_isClosed)
    {
        return;
    }

    // Restore the native window.
    Restore();

    // Hide the native window.
    [m_nsWindow orderOut : nil];
    m_isVisible = false;
}

//--------------------------------------------------------------
void WindowMacOS::Close()
{
    if (m_isClosed)
    {
        return;
    }

    // Hide the native window.
    Hide();

    // Close the native window.
    [m_nsWindow close];
    PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
void WindowMacOS::Maximize()
{
    if (IsMaximized() || !m_isVisible || m_isClosed)
    {
        return;
    }

    [m_nsWindow zoom: nil];
    while (IsMinimized())
    {
        [m_nsWindow deminiaturize: nil];
        PumpWindowEventsUntilEmpty();
    }
}

//--------------------------------------------------------------
void WindowMacOS::Minimize()
{
    if (IsMinimized() || !m_isVisible || m_isClosed)
    {
        return;
    }

    if (IsFullScreen())
    {
        Restore();
    }

    while (!IsMinimized())
    {
        [m_nsWindow miniaturize: nil];
        PumpWindowEventsUntilEmpty();
    }
}

//--------------------------------------------------------------
void WindowMacOS::Restore()
{
    if (!m_isVisible || m_isClosed)
    {
        return;
    }

    if (IsFullScreen())
    {
        FullScreenDisable();
    }

    if (IsMaximized())
    {
        [m_nsWindow zoom: nil];
    }

    while (IsMinimized())
    {
        [m_nsWindow deminiaturize: nil];
        PumpWindowEventsUntilEmpty();
    }
}

//--------------------------------------------------------------
void WindowMacOS::FullScreenEnable()
{
    if (IsFullScreen() || !m_isVisible || m_isClosed)
    {
        return;
    }

    Maximize();
    FullScreenToggle();
}

//--------------------------------------------------------------
void WindowMacOS::FullScreenDisable()
{
    if (!IsFullScreen() || !m_isVisible || m_isClosed)
    {
        return;
    }

    FullScreenToggle();
}

//--------------------------------------------------------------
void WindowMacOS::FullScreenToggle()
{
    assert(!m_isTransitioning);
    m_isTransitioning = true;
    [m_nsWindow toggleFullScreen: nil];
    while (m_isTransitioning)
    {
        PumpWindowEventsUntilEmpty();
    }
}

//--------------------------------------------------------------
void WindowMacOS::PumpWindowEventsOnce()
{
    @autoreleasepool
    {
        if (NSEvent* event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                                untilDate: [NSDate distantPast]
                                                   inMode: NSDefaultRunLoopMode
                                                  dequeue: YES])
        {
            [NSApp sendEvent: event];
        }
    }
}

//--------------------------------------------------------------
void WindowMacOS::PumpWindowEventsUntilEmpty()
{
    @autoreleasepool
    {
        while (NSEvent* event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                                   untilDate: [NSDate distantPast]
                                                      inMode: NSDefaultRunLoopMode
                                                     dequeue: YES])
        {
            m_nativeInputEvents.Dispatch(event);
            [NSApp sendEvent: event];
        }
    }
}

//--------------------------------------------------------------
bool WindowMacOS::IsFullScreen() const
{
    return m_nsWindow.styleMask & NSWindowStyleMaskFullScreen;
}

//--------------------------------------------------------------
bool WindowMacOS::IsMaximized() const
{
    return m_nsWindow.isZoomed && !m_nsWindow.isMiniaturized;
}

//--------------------------------------------------------------
bool WindowMacOS::IsMinimized() const
{
    return m_nsWindow.isMiniaturized;
}

//--------------------------------------------------------------
bool WindowMacOS::IsVisible() const
{
    return m_isVisible;
}

//--------------------------------------------------------------
bool WindowMacOS::IsClosed() const
{
    return m_isClosed;
}

//--------------------------------------------------------------
void WindowMacOS::GetDisplayDimensions(uint32_t& o_width,
                                       uint32_t& o_height) const
{
    NSRect windowRect = m_nsWindow.frame;
    NSRect displayRect = [m_nsWindow contentRectForFrameRect: windowRect];
    o_width = displayRect.size.width;
    o_height = displayRect.size.height;
}

//--------------------------------------------------------------
void WindowMacOS::GetWindowDimensions(uint32_t& o_width,
                                      uint32_t& o_height) const
{
    NSRect windowRect = m_nsWindow.frame;
    o_width = windowRect.size.width;
    o_height = windowRect.size.height;
}

//--------------------------------------------------------------
void* WindowMacOS::GetNativeDisplayHandle() const
{
    return nullptr;
}

//--------------------------------------------------------------
void* WindowMacOS::GetNativeWindowHandle() const
{
    return m_nsWindow;
}

//--------------------------------------------------------------
Window::NativeInputEvents* WindowMacOS::GetNativeInputEvents()
{
    return &m_nativeInputEvents;
}

//--------------------------------------------------------------
Window::NativeTextEvents* WindowMacOS::GetNativeTextEvents()
{
    return nullptr;
}

//--------------------------------------------------------------
void WindowMacOS::OnNativeWindowDidEnterFullScreen()
{
    m_isTransitioning = false;
}

//--------------------------------------------------------------
void WindowMacOS::OnNativeWindowDidExitFullScreen()
{
    m_isTransitioning = false;
}

//--------------------------------------------------------------
void WindowMacOS::OnNativeWindowWillClose()
{
    m_isClosed = true;
}

//--------------------------------------------------------------
@implementation WindowDelegate
{
    WindowMacOS* m_window;
}

//--------------------------------------------------------------
-(instancetype)initWithWindow: (WindowMacOS*) a_window
{
    if (self = [super init])
    {
        m_window = a_window;
    }
    return self;
}

//--------------------------------------------------------------
- (void)windowDidEnterFullScreen: (NSNotification *)notification
{
    m_window->OnNativeWindowDidEnterFullScreen();
}

//--------------------------------------------------------------
- (void)windowDidExitFullScreen: (NSNotification *)notification
{
    m_window->OnNativeWindowDidExitFullScreen();
}

//--------------------------------------------------------------
- (void)windowWillClose: (NSNotification*) notification
{
    m_window->OnNativeWindowWillClose();
}
@end
