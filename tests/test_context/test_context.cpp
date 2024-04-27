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
    bool bufferQuadrants = true;
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

    template<typename BufferType>
    void CycleColors(const BufferType* const a_colors[4],
                     int a_numColors);

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
        default: graphicsAPI = "NONE"; break;
    }

    string format;
    switch (a_contextConfig.bufferConfig.format)
    {
        case Buffer::Format::RGBA_FLOAT: format = "Format::RGBA_FLOAT"; break;
        case Buffer::Format::RGBA_UINT8: format = "Format::RGBA_UINT8"; break;
        case Buffer::Format::RGBA_UINT16: format = "Format::RGBA_UINT16"; break;
        default: format = "NONE"; break;
    }

    a_contextConfig.windowConfig.titleUTF8 = graphicsAPI + " " + format;
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
            static constexpr float RED[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            static constexpr float GREEN[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
            static constexpr float BLUE[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
            static constexpr float BLACK[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            static constexpr const float* COLORS[4] = { RED, GREEN, BLUE, BLACK };
            CycleColors<float>(COLORS, 4);
        }
        break;
        case Buffer::Format::RGBA_UINT8:
        {
            static constexpr uint8_t RED[4] = { 255, 0, 0, 255 };
            static constexpr uint8_t GREEN[4] = { 0, 255, 0, 255 };
            static constexpr uint8_t BLUE[4] = { 0, 0, 255, 255 };
            static constexpr uint8_t BLACK[4] = { 0, 0, 0, 255 };
            static constexpr const uint8_t* COLORS[4] = { RED, GREEN, BLUE, BLACK };
            CycleColors<uint8_t>(COLORS, 4);
        }
        break;
        case Buffer::Format::RGBA_UINT16:
        {
            static constexpr uint16_t MAX = numeric_limits<uint16_t>::max();
            static constexpr uint16_t RED[4] = { MAX, 0, 0, MAX };
            static constexpr uint16_t GREEN[4] = { 0, MAX, 0, MAX };
            static constexpr uint16_t BLUE[4] = { 0, 0, MAX, MAX };
            static constexpr uint16_t BLACK[4] = { 0, 0, 0, MAX };
            static constexpr const uint16_t* COLORS[4] = { RED, GREEN, BLUE, BLACK };
            CycleColors<uint16_t>(COLORS, 4);
        }
        break;
        default:
        {
        }
        break;
    }
}

//--------------------------------------------------------------
template<typename BufferType>
void TestApplication::CycleColors(const BufferType* const a_colors[4],
                                  int a_numColors)
{
    Buffer& buffer = m_context->GetBuffer();
    BufferType* pixelBuffer = buffer.GetData<BufferType>();
    if (!pixelBuffer)
    {
        return;
    }

    if (m_testParams.bufferQuadrants)
    {
        // Change the color of each quadrant every second.
        const int topLeftIndex = (int)m_secondsElapsed % 4;
        const int topRightIndex = (topLeftIndex == 3) ? 0 : topLeftIndex + 1;
        const int bottomLeftIndex = (topRightIndex == 3) ? 0 : topRightIndex + 1;
        const int bottomRightIndex = (bottomLeftIndex == 3) ? 0 : bottomLeftIndex + 1;
        const BufferType* colorTopLeft = a_colors[topLeftIndex];
        const BufferType* colorTopRight = a_colors[topRightIndex];
        const BufferType* colorBottomLeft = a_colors[bottomLeftIndex];
        const BufferType* colorBottomRight = a_colors[bottomRightIndex];

        const uint32_t pixelWidth = buffer.GetWidth();
        const uint32_t pixelHeight = buffer.GetHeight();
        const uint32_t numChannels = Buffer::ChannelsPerPixel(buffer.GetFormat());
        for (uint32_t y = 0; y < pixelHeight; ++y)
        {
            for (uint32_t x = 0; x < pixelWidth; ++x)
            {
                const uint32_t quadrant = (x > (pixelWidth / 2)) + (2 * (y > (pixelHeight / 2)));
                const BufferType* color = colorTopLeft;
                switch (quadrant)
                {
                    case 0: color = colorBottomLeft; break;
                    case 1: color = colorBottomRight; break;
                    case 2: color = colorTopLeft; break;
                    case 3: color = colorTopRight; break;
                }
                const uint32_t i = (x * numChannels) + (y * pixelWidth * numChannels);
                for (uint32_t z = 0; z < numChannels; ++z)
                {
                    pixelBuffer[i + z] = color[z];
                }
            }
        }
    }
    else
    {
        // Change the color of the entire buffer every second.
        const uint32_t index = (uint32_t)m_secondsElapsed % a_numColors;
        const BufferType* color = a_colors[index];

        const uint32_t numChannelsPerPixel = Buffer::ChannelsPerPixel(buffer.GetFormat());
        const uint32_t totalPixels = buffer.GetWidth() * buffer.GetHeight();
        const uint32_t totalChannels = totalPixels * numChannelsPerPixel;
        for (uint32_t i = 0; i < totalChannels; i += numChannelsPerPixel)
        {
            for (uint32_t j = 0; j < numChannelsPerPixel; ++j)
            {
                pixelBuffer[i + j] = color[j];
            }
        }
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
TEST_CASE("Test Context RGBA_FLOAT", "[context][rgba_float]")
{
    TestParams testParams;
    testParams.contextConfig.bufferConfig.format = Buffer::Format::RGBA_FLOAT;

    TestApplication testApplication(testParams);
    testApplication.Run();
}

//--------------------------------------------------------------
TEST_CASE("Test Context RGBA_UINT8", "[context][rgba_uint8]")
{
    TestParams testParams;
    testParams.contextConfig.bufferConfig.format = Buffer::Format::RGBA_UINT8;

    TestApplication testApplication(testParams);
    testApplication.Run();
}

//--------------------------------------------------------------
TEST_CASE("Test Context RGBA_UINT16", "[context][rgba_uint16]")
{
    TestParams testParams;
    testParams.contextConfig.bufferConfig.format = Buffer::Format::RGBA_UINT16;

    TestApplication testApplication(testParams);
    testApplication.Run();
}

//--------------------------------------------------------------
TEST_CASE("Test Context Thread", "[context][thread]")
{
    TestParams testParams;

    testParams.contextConfig.bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    testParams.secondsToRunFor = 3.0f;
    TestApplication testApplication1(testParams);

    testParams.contextConfig.bufferConfig.format = Buffer::Format::RGBA_UINT8;
    testParams.secondsToRunFor = 4.5f;
    TestApplication testApplication2(testParams);

    testParams.contextConfig.bufferConfig.format = Buffer::Format::RGBA_UINT16;
    testParams.secondsToRunFor = 6.0f;
    TestApplication testApplication3(testParams);

    thread runThread1 = testApplication1.RunInThread();
    thread runThread2 = testApplication2.RunInThread();
    thread runThread3 = testApplication3.RunInThread();

    runThread1.join();
    runThread2.join();
    runThread3.join();
}

//--------------------------------------------------------------
TEST_CASE("Test Context Graphics API", "[context][graphics_api]")
{
    TestParams testParams;

    testParams.contextConfig.graphicsAPI = Context::GraphicsAPI::NATIVE;
    testParams.secondsToRunFor = 2.0f;
    TestApplication testApplication1(testParams);

    testParams.contextConfig.graphicsAPI = Context::GraphicsAPI::OPENGL;
    testParams.secondsToRunFor = 2.0f;
    TestApplication testApplication2(testParams);

    testParams.contextConfig.graphicsAPI = Context::GraphicsAPI::VULKAN;
    testParams.secondsToRunFor = 2.0f;
    TestApplication testApplication3(testParams);

    // MacOS display contexts can only run on the main thread.
#ifdef __APPLE__
    testApplication1.Run();
    testApplication2.Run();
    testApplication3.Run();
#else
    thread runThread1 = testApplication1.RunInThread();
    thread runThread2 = testApplication2.RunInThread();
    thread runThread3 = testApplication3.RunInThread();

    runThread1.join();
    runThread2.join();
    runThread3.join();
#endif // __APPLE__
}
