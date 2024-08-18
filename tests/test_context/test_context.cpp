//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <simple/application/application.h>
#include <simple/display/context.h>
#include <catch2/catch.hpp>
#include <inttypes.h>

using namespace Simple::Display;
using namespace std;

//--------------------------------------------------------------
struct TestParams
{
    Context::Config contextConfig;
    float secondsToRunFor = 5.0f;
    bool cappedTargetFPS = false;
    bool printFrameStats = false;
    bool printAtShutDown = true;
};

//--------------------------------------------------------------
class TestApplication : public Simple::Application
{
public:
    TestApplication(const TestParams& a_testParams);
    ~TestApplication() override = default;

protected:
    void StartUp() override;
    void ShutDown() override;

    void UpdateStart(float a_deltaTimeSeconds) override;
    void UpdateFixed(float a_fixedTimeSeconds) override;
    void UpdateEnded(float a_deltaTimeSeconds) override;
    void UpdatePixelBuffer();

    void OnFrameComplete(const FrameStats& a_stats) override;

private:
    static void PrintFrameStats(const FrameStats& a_stats,
                                bool a_atShutDown = false);

    template<typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
    void CycleColors(const DataType a_colors[NumColors][ChannelsPerPixel]);

    const TestParams m_testParams;
    Context* m_context = nullptr;
    float m_secondsElapsed = 0.0f;
    FrameStats m_frameStats = {};
};

//--------------------------------------------------------------
void SetWindowTitle(Context::Config& a_contextConfig)
{
    string graphicsAPI;
    switch (a_contextConfig.graphicsAPI)
    {
        case Context::GraphicsAPI::NATIVE: graphicsAPI = "GraphicsAPI::NATIVE"; break;
        case Context::GraphicsAPI::OPENGL: graphicsAPI = "GraphicsAPI::OPENGL"; break;
        case Context::GraphicsAPI::VULKAN: graphicsAPI = "GraphicsAPI::VULKAN"; break;
        default: graphicsAPI = "GraphicsAPI::NONE"; break;
    }

    string format;
    switch (a_contextConfig.bufferConfig.format)
    {
        case Buffer::Format::RGBA_FLOAT: format = "Format::RGBA_FLOAT"; break;
        case Buffer::Format::RGBA_UINT8: format = "Format::RGBA_UINT8"; break;
        case Buffer::Format::RGBA_UINT16: format = "Format::RGBA_UINT16"; break;
        default: format = "Format::NONE"; break;
    }

    string interop;
    switch (a_contextConfig.bufferConfig.interop)
    {
        case Buffer::Interop::HOST: interop = "Interop::HOST"; break;
        case Buffer::Interop::CUDA: interop = "Interop::CUDA"; break;
        default: interop = "Interop::NONE"; break;
    }

    const string title = graphicsAPI + " " + format + " " + interop;
    a_contextConfig.windowConfig.titleUTF8 = title;
}

//--------------------------------------------------------------
TestApplication::TestApplication(const TestParams& a_testParams)
    : m_testParams(a_testParams)
{
    SetCappedFPS(m_testParams.cappedTargetFPS);

    if (m_testParams.contextConfig.windowConfig.titleUTF8 == DEFAULT_WINDOW_TITLE)
    {
        SetWindowTitle(const_cast<TestParams&>(m_testParams).contextConfig);
    }
}

//--------------------------------------------------------------
void TestApplication::StartUp()
{
    // Create the display context.
    m_context = new Context(m_testParams.contextConfig);

    // Shut down immediately if there is no display buffer.
    if (!m_context->GetBuffer().GetData())
    {
        RequestShutDown();
    }
}

//--------------------------------------------------------------
void TestApplication::ShutDown()
{
    // Destroy the display context.
    delete(m_context);
    m_context = nullptr;

    if (m_testParams.printAtShutDown)
    {
        PrintFrameStats(m_frameStats, true);
        m_frameStats = {};
    }
}

//--------------------------------------------------------------
void TestApplication::UpdateStart(float a_deltaTimeSeconds)
{
    (void)a_deltaTimeSeconds;
    m_context->OnFrameStart();
}

//--------------------------------------------------------------
void TestApplication::UpdateFixed(float a_fixedTimeSeconds)
{
    m_secondsElapsed += a_fixedTimeSeconds;

    // Update the pixel buffer every fixed update.
    UpdatePixelBuffer();

    // Shut down after five seconds worth of fixed updates.
    // Could be greater than five seconds of regular time
    // if updates are taking longer than the fixed time.
    if (m_secondsElapsed > m_testParams.secondsToRunFor)
    {
        RequestShutDown();
    }
}

//--------------------------------------------------------------
void TestApplication::UpdateEnded(float a_deltaTimeSeconds)
{
    (void)a_deltaTimeSeconds;
    m_context->OnFrameEnded();
}

//--------------------------------------------------------------
void TestApplication::UpdatePixelBuffer()
{
    switch (m_context->GetBuffer().GetFormat())
    {
        case Buffer::Format::RGBA_FLOAT:
        {
            constexpr float COLORS[4][4] = { { 1.0f, 0.0f, 0.0f, 1.0f },
                                             { 0.0f, 1.0f, 0.0f, 1.0f },
                                             { 0.0f, 0.0f, 1.0f, 1.0f },
                                             { 0.0f, 0.0f, 0.0f, 1.0f } };
            CycleColors<float, 4, 4>(COLORS);
        }
        break;
        case Buffer::Format::RGBA_UINT8:
        {
            constexpr uint8_t COLORS[4][4] = { { UINT8_MAX, 0, 0, UINT8_MAX },
                                               { 0, UINT8_MAX, 0, UINT8_MAX },
                                               { 0, 0, UINT8_MAX, UINT8_MAX },
                                               { 0, 0, 0, UINT8_MAX } };
            CycleColors<uint8_t, 4, 4>(COLORS);
        }
        break;
        case Buffer::Format::RGBA_UINT16:
        {
            constexpr uint16_t COLORS[4][4] = { { UINT16_MAX, 0, 0, UINT16_MAX },
                                                { 0, UINT16_MAX, 0, UINT16_MAX },
                                                { 0, 0, UINT16_MAX, UINT16_MAX },
                                                { 0, 0, 0, UINT16_MAX } };
            CycleColors<uint16_t, 4, 4>(COLORS);
        }
        break;
        default:
        {
        }
        break;
    }
}

//--------------------------------------------------------------
template<typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
void CycleColorsHost(const DataType a_colors[NumColors][ChannelsPerPixel],
                     const Buffer& a_buffer,
                     float a_secondsElapsed)
{
    DataType* pixelBuffer = a_buffer.GetData<DataType>();
    if (!pixelBuffer)
    {
        return;
    }

    // Change the color of each quadrant every second.
    const uint32_t topLeftIndex = (uint32_t)a_secondsElapsed % NumColors;
    const uint32_t topRightIndex = (topLeftIndex == NumColors - 1) ? 0 : min(topLeftIndex + 1, NumColors);
    const uint32_t bottomLeftIndex = (topRightIndex == NumColors - 1) ? 0 : min(topRightIndex + 1, NumColors);
    const uint32_t bottomRightIndex = (bottomLeftIndex == NumColors - 1) ? 0 : min(bottomLeftIndex + 1, NumColors);

    const DataType* colorTopLeft = a_colors[topLeftIndex];
    const DataType* colorTopRight = a_colors[topRightIndex];
    const DataType* colorBottomLeft = a_colors[bottomLeftIndex];
    const DataType* colorBottomRight = a_colors[bottomRightIndex];

    const uint32_t pixelWidth = a_buffer.GetWidth();
    const uint32_t pixelHeight = a_buffer.GetHeight();
    assert(ChannelsPerPixel == Buffer::ChannelsPerPixel(a_buffer.GetFormat()));
    for (uint32_t y = 0; y < pixelHeight; ++y)
    {
        for (uint32_t x = 0; x < pixelWidth; ++x)
        {
            const uint32_t quadrant = (x > (pixelWidth / 2)) + (2 * (y > (pixelHeight / 2)));
            const DataType* color = colorTopLeft;
            switch (quadrant)
            {
                case 0: color = colorBottomLeft; break;
                case 1: color = colorBottomRight; break;
                case 2: color = colorTopLeft; break;
                case 3: color = colorTopRight; break;
            }
            const uint32_t i = (x * ChannelsPerPixel) + (y * pixelWidth * ChannelsPerPixel);
            for (uint32_t z = 0; z < ChannelsPerPixel; ++z)
            {
                pixelBuffer[i + z] = color[z];
            }
        }
    }
}

//--------------------------------------------------------------
#ifdef CUDA_SUPPORTED
template<typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
extern void CycleColorsCuda(const DataType a_colors[NumColors][ChannelsPerPixel],
                            const Buffer& a_buffer,
                            float a_secondsElapsed);
#endif // CUDA_SUPPORTED

//--------------------------------------------------------------
template<typename DataType, uint32_t ChannelsPerPixel, uint32_t NumColors>
void TestApplication::CycleColors(const DataType a_colors[NumColors][ChannelsPerPixel])
{
    const Buffer& buffer = m_context->GetBuffer();
    if (buffer.GetInterop() == Buffer::Interop::HOST)
    {
        CycleColorsHost<DataType, ChannelsPerPixel, NumColors>(a_colors, buffer, m_secondsElapsed);
    }
    else if (buffer.GetInterop() == Buffer::Interop::CUDA)
    {
    #ifdef CUDA_SUPPORTED
        CycleColorsCuda<DataType, ChannelsPerPixel, NumColors>(a_colors, buffer, m_secondsElapsed);
    #endif // CUDA_SUPPORTED
    }
}

//--------------------------------------------------------------
void TestApplication::OnFrameComplete(const FrameStats& a_stats)
{
    m_frameStats = a_stats;
    if (m_testParams.printFrameStats)
    {
        PrintFrameStats(m_frameStats);
    }
}

//--------------------------------------------------------------
void TestApplication::PrintFrameStats(const FrameStats& a_stats,
                                      bool a_atShutDown)
{
    auto toMs = [](const Duration& a_duration)->int64_t
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(a_duration).count();
    };

    if (a_atShutDown)
    {
        printf( "Frame Count:   %" PRIu64 "\n"
                "Average FPS:   %" PRIu32 "\n"
                "Target FPS:    %" PRIu32 "\n"
                "Total Dur:     %" PRIi64 " (ms)\n"
                "\n",
                a_stats.frameCount,
                a_stats.averageFPS,
                a_stats.targetFPS,
                toMs(a_stats.totalDur));
    }
    else
    {
        printf( "Frame Count:   %" PRIu64 "\n"
                "Average FPS:   %" PRIu32 "\n"
                "Target FPS:    %" PRIu32 "\n"
                "Actual Dur:    %" PRIi64 " (ms)\n"
                "Target Dur:    %" PRIi64 " (ms)\n"
                "Excess Dur:    %" PRIi64 " (ms)\n"
                "Total Dur:     %" PRIi64 " (ms)\n"
                "\n",
                a_stats.frameCount,
                a_stats.averageFPS,
                a_stats.targetFPS,
                toMs(a_stats.actualDur),
                toMs(a_stats.targetDur),
                toMs(a_stats.excessDur),
                toMs(a_stats.totalDur));
    }
}

//--------------------------------------------------------------
void TestContext(TestParams a_testParams)
{
    Context::Config& contextConfig = a_testParams.contextConfig;
    Buffer::Config& bufferConfig = contextConfig.bufferConfig;
    SECTION("Format::RGBA_FLOAT")
    {
        bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    }
    SECTION("Format::RGBA_UINT8")
    {
        bufferConfig.format = Buffer::Format::RGBA_UINT8;
    }
    SECTION("Format::RGBA_UINT16")
    {
        bufferConfig.format = Buffer::Format::RGBA_UINT16;
    }

    TestApplication testApplication(a_testParams);
    testApplication.Run();
}

//--------------------------------------------------------------
TEST_CASE("Test Context Native Host", "[context][native][host]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::NATIVE;
    contextConfig.bufferConfig.interop = Buffer::Interop::HOST;
    TestContext(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context Native CUDA", "[context][native][cuda]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::NATIVE;
    contextConfig.bufferConfig.interop = Buffer::Interop::CUDA;
    TestContext(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context OpenGL Host", "[context][opengl][host]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::OPENGL;
    contextConfig.bufferConfig.interop = Buffer::Interop::HOST;
    TestContext(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context OpenGL CUDA", "[context][opengl][cuda]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::OPENGL;
    contextConfig.bufferConfig.interop = Buffer::Interop::CUDA;
    TestContext(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context Vulkan Host", "[context][vulkan][host]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::VULKAN;
    contextConfig.bufferConfig.interop = Buffer::Interop::HOST;
    TestContext(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context Vulkan CUDA", "[context][vulkan][cuda]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::VULKAN;
    contextConfig.bufferConfig.interop = Buffer::Interop::CUDA;
    TestContext(testParams);
}

//--------------------------------------------------------------
void TestContextThreads(TestParams a_testParams)
{
    Context::Config& contextConfig = a_testParams.contextConfig;
    Buffer::Config& bufferConfig = contextConfig.bufferConfig;
    Window::Config& windowConfig = contextConfig.windowConfig;

    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    bufferConfig.interop = Buffer::Interop::HOST;
    windowConfig.initialPositionX = 100;
    windowConfig.initialPositionY = 100;
    a_testParams.secondsToRunFor = 1.0f;
    TestApplication testApplication1(a_testParams);

    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    bufferConfig.interop = Buffer::Interop::CUDA;
    windowConfig.initialPositionX = 200;
    windowConfig.initialPositionY = 200;
    a_testParams.secondsToRunFor = 2.0f;
    TestApplication testApplication2(a_testParams);

    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    bufferConfig.interop = Buffer::Interop::HOST;
    windowConfig.initialPositionX = 300;
    windowConfig.initialPositionY = 300;
    a_testParams.secondsToRunFor = 3.0f;
    TestApplication testApplication3(a_testParams);

    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    bufferConfig.interop = Buffer::Interop::CUDA;
    windowConfig.initialPositionX = 400;
    windowConfig.initialPositionY = 400;
    a_testParams.secondsToRunFor = 4.0f;
    TestApplication testApplication4(a_testParams);

    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    bufferConfig.interop = Buffer::Interop::HOST;
    windowConfig.initialPositionX = 500;
    windowConfig.initialPositionY = 500;
    a_testParams.secondsToRunFor = 5.0f;
    TestApplication testApplication5(a_testParams);

    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    bufferConfig.interop = Buffer::Interop::CUDA;
    windowConfig.initialPositionX = 600;
    windowConfig.initialPositionY = 600;
    a_testParams.secondsToRunFor = 6.0f;
    TestApplication testApplication6(a_testParams);

    thread runThread1 = testApplication1.RunInThread();
    thread runThread2 = testApplication2.RunInThread();
    thread runThread3 = testApplication3.RunInThread();
    thread runThread4 = testApplication4.RunInThread();
    thread runThread5 = testApplication5.RunInThread();
    thread runThread6 = testApplication6.RunInThread();

    runThread1.join();
    runThread2.join();
    runThread3.join();
    runThread4.join();
    runThread5.join();
    runThread6.join();
}

//--------------------------------------------------------------
TEST_CASE("Test Context Native Threads", "[context][native][threads]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::NATIVE;
    TestContextThreads(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context OpenGL Threads", "[context][opengl][threads]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::OPENGL;
    TestContextThreads(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context Vulkan Threads", "[context][vulkan][threads]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    contextConfig.graphicsAPI = Context::GraphicsAPI::VULKAN;
    TestContextThreads(testParams);
}

//--------------------------------------------------------------
TEST_CASE("Test Context All Threads", "[context][all][threads]")
{
    TestParams testParams;
    Context::Config& contextConfig = testParams.contextConfig;
    Buffer::Config& bufferConfig = contextConfig.bufferConfig;
    Window::Config& windowConfig = contextConfig.windowConfig;

    contextConfig.graphicsAPI = Context::GraphicsAPI::NATIVE;
    bufferConfig.interop = Buffer::Interop::HOST;
    windowConfig.initialPositionX = 100;
    windowConfig.initialPositionY = 100;
    TestApplication testApplication1(testParams);

    contextConfig.graphicsAPI = Context::GraphicsAPI::OPENGL;
    bufferConfig.interop = Buffer::Interop::HOST;
    windowConfig.initialPositionX = 200;
    windowConfig.initialPositionY = 200;
    TestApplication testApplication2(testParams);

    contextConfig.graphicsAPI = Context::GraphicsAPI::VULKAN;
    bufferConfig.interop = Buffer::Interop::HOST;
    windowConfig.initialPositionX = 300;
    windowConfig.initialPositionY = 300;
    TestApplication testApplication3(testParams);

    contextConfig.graphicsAPI = Context::GraphicsAPI::NATIVE;
    bufferConfig.interop = Buffer::Interop::CUDA;
    windowConfig.initialPositionX = 400;
    windowConfig.initialPositionY = 400;
    TestApplication testApplication4(testParams);

    contextConfig.graphicsAPI = Context::GraphicsAPI::OPENGL;
    bufferConfig.interop = Buffer::Interop::CUDA;
    windowConfig.initialPositionX = 500;
    windowConfig.initialPositionY = 500;
    TestApplication testApplication5(testParams);

    contextConfig.graphicsAPI = Context::GraphicsAPI::VULKAN;
    bufferConfig.interop = Buffer::Interop::CUDA;
    windowConfig.initialPositionX = 600;
    windowConfig.initialPositionY = 600;
    TestApplication testApplication6(testParams);

    thread runThread1 = testApplication1.RunInThread();
    thread runThread2 = testApplication2.RunInThread();
    thread runThread3 = testApplication3.RunInThread();
    thread runThread4 = testApplication4.RunInThread();
    thread runThread5 = testApplication5.RunInThread();
    thread runThread6 = testApplication6.RunInThread();

    runThread1.join();
    runThread2.join();
    runThread3.join();
    runThread4.join();
    runThread5.join();
    runThread6.join();
}
