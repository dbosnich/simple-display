//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <display/window_implementation.h>

#define NOMINMAX
#include <windows.h>
#include <codecvt>

using namespace Simple::Display;

//--------------------------------------------------------------
class UTF16ToUTF8Converter
{
public:
    std::string Feed(uint16_t a_codeUnitUTF16);

private:
    std::wstring_convert<std::codecvt_utf8<wchar_t>> m_converter;
    uint16_t m_leadSurrogate = 0;
};

//--------------------------------------------------------------
class WindowWin32 : public Window::Implementation
{
public:
    static constexpr const wchar_t* CLASS_NAME = L"SimpleWindow";
    static uint32_t s_instanceCount;

    WindowWin32(const Window::Config& a_config);
    ~WindowWin32() override;

    WindowWin32(const WindowWin32&) = delete;
    WindowWin32& operator=(const WindowWin32&) = delete;

    void OnNativeWindowDestroyed();
    void OnNativeDeviceEvent(WPARAM a_wParam);
    void OnNativeInputEvent(RAWINPUT* a_rawInput);
    void OnNativeTextEvent(const USHORT a_codeUnitUTF16);

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

    Window::NativeDeviceEvents* GetNativeDeviceEvents() override;
    Window::NativeInputEvents* GetNativeInputEvents() override;
    Window::NativeTextEvents* GetNativeTextEvents() override;

private:
    Window::NativeDeviceEvents m_nativeDeviceEvents;
    Window::NativeInputEvents m_nativeInputEvents;
    Window::NativeTextEvents m_nativeTextEvents;
    UTF16ToUTF8Converter m_utf16ToUtf8Converter;
    HWND m_windowHandle = nullptr;
    bool m_isFullScreen = false;
    bool m_isVisible = false;
    bool m_isClosed = false;
};

//--------------------------------------------------------------
using ImplPtr = std::unique_ptr<Window::Implementation>;
ImplPtr Window::Implementation::Create(const Config& a_config)
{
    return std::make_unique<WindowWin32>(a_config);
}

//--------------------------------------------------------------
LRESULT CALLBACK OnWindowMessage(HWND a_handle,
                                 UINT a_message,
                                 WPARAM a_wParam,
                                 LPARAM a_lParam);

//--------------------------------------------------------------
uint32_t WindowWin32::s_instanceCount = 0;

//--------------------------------------------------------------
WindowWin32::WindowWin32(const Window::Config& a_config)
{
    if (::InterlockedIncrement(&s_instanceCount) == 1)
    {
        // Register the native window class.
        WNDCLASSW windowClass;
        windowClass.style = 0;
        windowClass.lpfnWndProc = &OnWindowMessage;
        windowClass.cbClsExtra = 0;
        windowClass.cbWndExtra = 0;
        windowClass.hInstance = nullptr;
        windowClass.hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
        windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        windowClass.hbrBackground = 0;
        windowClass.lpszMenuName = nullptr;
        windowClass.lpszClassName = CLASS_NAME;
        ::RegisterClassW(&windowClass);
    }

    // Create the native window.
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> con;
    std::wstring titleUTF16 = con.from_bytes(a_config.titleUTF8);
    const int initialPositionX = a_config.initialPositionX ?
                                 a_config.initialPositionX :
                                 CW_USEDEFAULT;
    const int initialPositionY = a_config.initialPositionY ?
                                 a_config.initialPositionY :
                                 CW_USEDEFAULT;
    m_windowHandle = ::CreateWindowExW(WS_EX_APPWINDOW,
                                       CLASS_NAME,
                                       titleUTF16.c_str(),
                                       WS_OVERLAPPEDWINDOW,
                                       initialPositionX,
                                       initialPositionY,
                                       a_config.initialWidth,
                                       a_config.initialHeight,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       this);
}

//--------------------------------------------------------------
WindowWin32::~WindowWin32()
{
    // Close the native window.
    Close();

    // Unregister the native window class.
    if (::InterlockedDecrement(&s_instanceCount) == 0)
    {
        ::UnregisterClassW(CLASS_NAME, nullptr);
    }
}

//--------------------------------------------------------------
void WindowWin32::Show()
{
    if (m_isVisible || m_isClosed)
    {
        return;
    }

    // Show the native window.
    ::ShowWindow(m_windowHandle, SW_SHOW);
    m_isVisible = true;
}

//--------------------------------------------------------------
void WindowWin32::Hide()
{
    if (!m_isVisible || m_isClosed)
    {
        return;
    }

    // Restore the native window.
    Restore();

    // Hide the native window.
    ::ShowWindow(m_windowHandle, SW_HIDE);
    m_isVisible = false;
}

//--------------------------------------------------------------
void WindowWin32::Close()
{
    if (m_isClosed)
    {
        return;
    }

    // Hide the native window.
    Hide();

    // Destroy the native window.
    m_isClosed = ::DestroyWindow(m_windowHandle);
}

//--------------------------------------------------------------
void WindowWin32::Maximize()
{
    if (IsMaximized() || !m_isVisible || m_isClosed)
    {
        return;
    }

    ::SendMessageW(m_windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

//--------------------------------------------------------------
void WindowWin32::Minimize()
{
    if (IsMinimized() || !m_isVisible || m_isClosed)
    {
        return;
    }

    if (IsFullScreen())
    {
        FullScreenDisable();
    }

    ::SendMessageW(m_windowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

//--------------------------------------------------------------
void WindowWin32::Restore()
{
    if (!m_isVisible || m_isClosed)
    {
        return;
    }

    if (IsFullScreen())
    {
        FullScreenDisable();
    }
    else if (IsMaximized() || IsMinimized())
    {
        ::SendMessageW(m_windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
}

//--------------------------------------------------------------
void WindowWin32::FullScreenEnable()
{
    if (m_isFullScreen || !m_isVisible || m_isClosed)
    {
        return;
    }

    // Change the native window style.
    ::SetWindowLongW(m_windowHandle, GWL_STYLE, WS_POPUP);

    // Maximize the native window.
    ::SendMessageW(m_windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);

    m_isFullScreen = true;
}

//--------------------------------------------------------------
void WindowWin32::FullScreenDisable()
{
    if (!m_isFullScreen || !m_isVisible || m_isClosed)
    {
        return;
    }

    // Change the native window style back.
    ::SetWindowLongW(m_windowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);

    // Restore the native window.
    ::SendMessageW(m_windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);

    m_isFullScreen = false;
}

//--------------------------------------------------------------
void WindowWin32::FullScreenToggle()
{
    m_isFullScreen ? FullScreenDisable() : FullScreenEnable();
}

//--------------------------------------------------------------
void WindowWin32::PumpWindowEventsOnce()
{
    MSG msg;
    if (::PeekMessageW(&msg, m_windowHandle, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
}

//--------------------------------------------------------------
void WindowWin32::PumpWindowEventsUntilEmpty()
{
    MSG msg;
    while (::PeekMessageW(&msg, m_windowHandle, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
}

//--------------------------------------------------------------
bool WindowWin32::IsFullScreen() const
{
    return m_isFullScreen;
}

//--------------------------------------------------------------
bool WindowWin32::IsMaximized() const
{
    return ::IsZoomed(m_windowHandle);
}

//--------------------------------------------------------------
bool WindowWin32::IsMinimized() const
{
    return ::IsIconic(m_windowHandle);
}

//--------------------------------------------------------------
bool WindowWin32::IsVisible() const
{
    return m_isVisible;
}

//--------------------------------------------------------------
bool WindowWin32::IsClosed() const
{
    return m_isClosed;
}

//--------------------------------------------------------------
void WindowWin32::GetDisplayDimensions(uint32_t& o_width,
                                       uint32_t& o_height) const
{
    ::RECT dimensions;
    if (::GetClientRect(m_windowHandle, &dimensions))
    {
        o_width = dimensions.right - dimensions.left;
        o_height = dimensions.bottom - dimensions.top;
    }
}

//--------------------------------------------------------------
void WindowWin32::GetWindowDimensions(uint32_t& o_width,
                                      uint32_t& o_height) const
{
    ::RECT dimensions;
    if (::GetWindowRect(m_windowHandle, &dimensions))
    {
        o_width = dimensions.right - dimensions.left;
        o_height = dimensions.bottom - dimensions.top;
    }
}

//--------------------------------------------------------------
void* WindowWin32::GetNativeDisplayHandle() const
{
    return nullptr;
}

//--------------------------------------------------------------
void* WindowWin32::GetNativeWindowHandle() const
{
    return m_windowHandle;
}

//--------------------------------------------------------------
Window::NativeDeviceEvents* WindowWin32::GetNativeDeviceEvents()
{
    return &m_nativeDeviceEvents;
}

//--------------------------------------------------------------
Window::NativeInputEvents* WindowWin32::GetNativeInputEvents()
{
    return &m_nativeInputEvents;
}

//--------------------------------------------------------------
Window::NativeTextEvents* WindowWin32::GetNativeTextEvents()
{
    return &m_nativeTextEvents;
}

//--------------------------------------------------------------
void WindowWin32::OnNativeWindowDestroyed()
{
    m_isClosed = true;
}

//--------------------------------------------------------------
void WindowWin32::OnNativeDeviceEvent(WPARAM a_wParam)
{
    m_nativeDeviceEvents.Dispatch(&a_wParam);
}

//--------------------------------------------------------------
void WindowWin32::OnNativeInputEvent(RAWINPUT* a_rawInput)
{
    m_nativeInputEvents.Dispatch(a_rawInput);
}

//--------------------------------------------------------------
void WindowWin32::OnNativeTextEvent(const USHORT a_codeUnitUTF16)
{
    auto utf8 = m_utf16ToUtf8Converter.Feed(a_codeUnitUTF16);
    if (!utf8.empty())
    {
        m_nativeTextEvents.Dispatch(utf8);
    }
}

//--------------------------------------------------------------
LRESULT CALLBACK OnWindowMessage(HWND a_handle,
                                 UINT a_message,
                                 WPARAM a_wParam,
                                 LPARAM a_lParam)
{
    // The create message is called before user data can be set.
    // Passing 'this' to CreateWindowExW can make it accessible
    // from the create message where it can be set as user data,
    // in turn used to forward all messages to the right window.
    WindowWin32* window = nullptr;
    if (a_message == WM_NCCREATE)
    {
        const LPCREATESTRUCT param = (LPCREATESTRUCT)a_lParam;
        LONG_PTR userData = (LONG_PTR)param->lpCreateParams;
        ::SetWindowLongPtrW(a_handle, GWLP_USERDATA, userData);
        window = (WindowWin32*)userData;
    }
    else
    {
        LONG_PTR userData = ::GetWindowLongPtrW(a_handle,
                                                GWLP_USERDATA);
        window = (WindowWin32*)userData;
    }

    if (!window)
    {
        return ::DefWindowProcW(a_handle,
                                a_message,
                                a_wParam,
                                a_lParam);
    }

    switch (a_message)
    {
        case WM_CHAR:
        {
            const USHORT codeUnitUTF16 = (USHORT)(a_wParam);
            window->OnNativeTextEvent(codeUnitUTF16);
            return 0;
        }
        break;
        case WM_DEVICECHANGE:
        {
            window->OnNativeDeviceEvent(a_wParam);
            return 0;
        }
        break;
        case WM_DESTROY:
        {
            window->OnNativeWindowDestroyed();
            return 0;
        }
        break;
        case WM_INPUT:
        {
            UINT rawInputSize;
            HRAWINPUT hRawInput = (HRAWINPUT)a_lParam;
            const UINT headerSize = sizeof(RAWINPUTHEADER);
            ::GetRawInputData(hRawInput,
                                RID_INPUT,
                                NULL,
                                &rawInputSize,
                                headerSize);

            std::vector<BYTE> rawInputBytes(rawInputSize);
            LPBYTE rawInputBytesData = rawInputBytes.data();
            ::GetRawInputData(hRawInput,
                                RID_INPUT,
                                rawInputBytesData,
                                &rawInputSize,
                                headerSize);

            RAWINPUT* rawInput = (RAWINPUT*)rawInputBytesData;
            window->OnNativeInputEvent(rawInput);
            return 0;
        }
        break;
        default:
        {
            return ::DefWindowProcW(a_handle,
                                    a_message,
                                    a_wParam,
                                    a_lParam);
        }
        break;
    }
}

//--------------------------------------------------------------
std::string UTF16ToUTF8Converter::Feed(uint16_t a_codeUnitUTF16)
{
    std::string codePointUTF8;

    if (IS_HIGH_SURROGATE(a_codeUnitUTF16))
    {
        // Store the lead surrogate and wait for the trailing.
        m_leadSurrogate = a_codeUnitUTF16;
    }
    else if (IS_LOW_SURROGATE(a_codeUnitUTF16))
    {
        if (m_leadSurrogate)
        {
            // Convert UTF-16 surrogate pair to UTF-8 code point.
            const wchar_t codePointUTF16[2] = { m_leadSurrogate,
                                                a_codeUnitUTF16 };
            codePointUTF8 = m_converter.to_bytes(codePointUTF16,
                                                 codePointUTF16 + 2);
            m_leadSurrogate = 0;
        }
        else
        {
            // Encoding error
            m_leadSurrogate = 0;
        }
    }
    else
    {
        // Convert standalone UTF-16 code point to UTF-8 code point.
        const wchar_t codePointUTF16[1] = { a_codeUnitUTF16 };
        codePointUTF8 = m_converter.to_bytes(codePointUTF16,
                                             codePointUTF16 + 1);
        m_leadSurrogate = 0;
    }

    return codePointUTF8;
}
