//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <simple/display/window.h>
#include <catch2/catch.hpp>

using namespace Simple::Display;

//--------------------------------------------------------------
TEST_CASE("Test Window Default", "[window][default]")
{
    Window testWindow({});
}

//--------------------------------------------------------------
TEST_CASE("Test Window Config", "[window][config]")
{
    Window::Config config;
    Window testWindow(config);
}

//--------------------------------------------------------------
TEST_CASE("Test Window Name", "[window][name]")
{
    Window testWindow({ "Test Window" });
}

//--------------------------------------------------------------
TEST_CASE("Test Window Name Unicode", "[window][name][unicode]")
{
    Window testWindow({ u8"\u30c6\u30b9\u30c8" });
}

//--------------------------------------------------------------
TEST_CASE("Test Window Width Height", "[window][width][height]")
{
    Window testWindow({ "Test Window", 1920, 1080 });
}

//--------------------------------------------------------------
TEST_CASE("Test Window Show", "[window][show]")
{
    Window testWindow({});
    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
}

//--------------------------------------------------------------
TEST_CASE("Test Window Hide", "[window][hide]")
{
    Window testWindow({});
    testWindow.Show();
    REQUIRE(testWindow.IsVisible());

    testWindow.Hide();
    REQUIRE(!testWindow.IsVisible());
}

//--------------------------------------------------------------
TEST_CASE("Test Window Close", "[window][close]")
{
    Window testWindow({});
    testWindow.Close();
    REQUIRE(testWindow.IsClosed());
}

//--------------------------------------------------------------
TEST_CASE("Test Window Show Hide Close", "[window][show][hide][close]")
{
    Window testWindow({});
    testWindow.Show();
    REQUIRE(testWindow.IsVisible());

    testWindow.Hide();
    REQUIRE(!testWindow.IsVisible());

    testWindow.Show();
    REQUIRE(testWindow.IsVisible());

    testWindow.Close();
    REQUIRE(testWindow.IsClosed());
}

//--------------------------------------------------------------
TEST_CASE("Test Window Maximize", "[window][maximize]")
{
    Window testWindow({});
    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsMaximized());

    testWindow.Maximize();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(testWindow.IsMaximized());

    testWindow.Hide();
    REQUIRE(!testWindow.IsVisible());
    REQUIRE(!testWindow.IsMaximized());

    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsMaximized());

    testWindow.Maximize();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(testWindow.IsMaximized());

    testWindow.Close();
    REQUIRE(!testWindow.IsVisible());
    REQUIRE(!testWindow.IsMaximized());
}

//--------------------------------------------------------------
TEST_CASE("Test Window Minimize", "[window][minimize]")
{
    Window testWindow({});
    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsMinimized());

    testWindow.Minimize();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(testWindow.IsMinimized());

    testWindow.Hide();
    REQUIRE(!testWindow.IsVisible());
    REQUIRE(!testWindow.IsMinimized());

    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsMinimized());

    testWindow.Minimize();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(testWindow.IsMinimized());

    testWindow.Close();
    REQUIRE(!testWindow.IsVisible());
    REQUIRE(!testWindow.IsMinimized());
}

//--------------------------------------------------------------
TEST_CASE("Test Window Restore", "[window][restore]")
{
    Window testWindow({});
    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Maximize();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Minimize();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenEnable();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Maximize();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Minimize();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Minimize();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Maximize();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Maximize();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenEnable();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Minimize();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenEnable();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenEnable();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Minimize();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenEnable();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Maximize();
    REQUIRE(testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Restore();
    REQUIRE(!testWindow.IsMaximized());
    REQUIRE(!testWindow.IsMinimized());
    REQUIRE(!testWindow.IsFullScreen());
}

//--------------------------------------------------------------
TEST_CASE("Test Window FullScreen", "[window][fullscreen]")
{
    Window testWindow({});
    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenEnable();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.FullScreenDisable();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenToggle();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Hide();
    REQUIRE(!testWindow.IsVisible());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.Show();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(!testWindow.IsFullScreen());

    testWindow.FullScreenEnable();
    REQUIRE(testWindow.IsVisible());
    REQUIRE(testWindow.IsFullScreen());

    testWindow.Close();
    REQUIRE(!testWindow.IsVisible());
    REQUIRE(!testWindow.IsFullScreen());
}

//--------------------------------------------------------------
TEST_CASE("Test Window Events", "[window][events]")
{
    Window testWindow({});
    testWindow.PumpWindowEventsOnce();
    testWindow.PumpWindowEventsUntilEmpty();
}

//--------------------------------------------------------------
TEST_CASE("Test Window Dimensions", "[window][dimensions]")
{
    Window::Config config;
    config.initialWidth = 1920;
    config.initialHeight = 1080;
    Window testWindow(config);

    uint32_t windowWidth = 0;
    uint32_t windowHeight = 0;
    testWindow.GetWindowDimensions(windowWidth, windowHeight);

    REQUIRE(windowWidth > 0);
    REQUIRE(windowHeight > 0);
    REQUIRE(windowWidth == config.initialWidth);
    REQUIRE(windowHeight == config.initialHeight);
}

//--------------------------------------------------------------
TEST_CASE("Test Window Display Dimensions", "[window][display][dimensions]")
{
    Window::Config config;
    config.initialWidth = 1920;
    config.initialHeight = 1080;
    Window testWindow(config);

    uint32_t displayWidth = 0;
    uint32_t displayHeight = 0;
    testWindow.GetDisplayDimensions(displayWidth, displayHeight);

    REQUIRE(displayWidth > 0);
    REQUIRE(displayHeight > 0);
    REQUIRE(displayWidth <= config.initialWidth);
    REQUIRE(displayHeight <= config.initialHeight);
}

//--------------------------------------------------------------
TEST_CASE("Test Window Multiple", "[window][multiple]")
{
    Window testWindow1({ "Window 1" });
    Window testWindow2({ "Window 2" });

    testWindow1.Show();
    testWindow2.Show();

    // Change to true to test interacting with the two windows.
    bool waitForUserToClose = false;

    bool closed1 = false;
    bool closed2 = false;
    do
    {
        if (!closed1)
        {
            testWindow1.PumpWindowEventsUntilEmpty();
            closed1 = testWindow1.IsClosed();
        }
        if (!closed2)
        {
            testWindow2.PumpWindowEventsUntilEmpty();
            closed2 = testWindow2.IsClosed();
        }
    }
    while (waitForUserToClose && !(closed1 && closed2));

    testWindow2.Hide();
    testWindow1.Hide();
}
