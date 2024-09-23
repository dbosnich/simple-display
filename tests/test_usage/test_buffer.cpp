//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <simple/display/buffer.h>
#include <simple/display/context.h>
#include <catch2/catch.hpp>

using namespace Simple::Display;

//--------------------------------------------------------------
inline void RequireBufferValues(const Buffer& a_buffer,
                                const Buffer::Config& a_config)
{
    const uint32_t minSize = Buffer::MinSizeBytes(a_config);
    const uint32_t minPitch = Buffer::MinPitchBytes(a_config);

    REQUIRE(a_buffer.GetData());
    REQUIRE(a_buffer.GetSize() >= minSize);
    REQUIRE(a_buffer.GetPitch() >= minPitch);
    REQUIRE(a_buffer.GetWidth() == a_config.width);
    REQUIRE(a_buffer.GetHeight() == a_config.height);
    REQUIRE(a_buffer.GetFormat() == a_config.format);
    REQUIRE(a_buffer.GetInterop() == a_config.interop);

    switch (a_config.format)
    {
        case Buffer::Format::RGBA_FLOAT:
        {
            if (a_buffer.GetInterop() == Buffer::Interop::HOST)
            {
                REQUIRE(a_buffer.GetData<float>());
                REQUIRE(!a_buffer.GetData<float, Buffer::Interop::CUDA>());
            }
            else
            {
                REQUIRE(!a_buffer.GetData<float>());
                REQUIRE(a_buffer.GetData<float, Buffer::Interop::CUDA>());
            }
            REQUIRE(!a_buffer.GetData<uint8_t>());
            REQUIRE(!a_buffer.GetData<uint16_t>());
            REQUIRE(!a_buffer.GetData<uint8_t, Buffer::Interop::CUDA>());
            REQUIRE(!a_buffer.GetData<uint16_t, Buffer::Interop::CUDA>());
        }
        break;
        case Buffer::Format::RGBA_UINT8:
        {
            if (a_buffer.GetInterop() == Buffer::Interop::HOST)
            {
                REQUIRE(a_buffer.GetData<uint8_t>());
                REQUIRE(!a_buffer.GetData<uint8_t, Buffer::Interop::CUDA>());
            }
            else
            {
                REQUIRE(!a_buffer.GetData<uint8_t>());
                REQUIRE(a_buffer.GetData<uint8_t, Buffer::Interop::CUDA>());
            }
            REQUIRE(!a_buffer.GetData<float>());
            REQUIRE(!a_buffer.GetData<uint16_t>());
            REQUIRE(!a_buffer.GetData<float, Buffer::Interop::CUDA>());
            REQUIRE(!a_buffer.GetData<uint16_t, Buffer::Interop::CUDA>());
        }
        break;
        case Buffer::Format::RGBA_UINT16:
        {
            if (a_buffer.GetInterop() == Buffer::Interop::HOST)
            {
                REQUIRE(a_buffer.GetData<uint16_t>());
                REQUIRE(!a_buffer.GetData<uint16_t, Buffer::Interop::CUDA>());
            }
            else
            {
                REQUIRE(!a_buffer.GetData<uint16_t>());
                REQUIRE(a_buffer.GetData<uint16_t, Buffer::Interop::CUDA>());
            }
            REQUIRE(!a_buffer.GetData<float>());
            REQUIRE(!a_buffer.GetData<uint8_t>());
            REQUIRE(!a_buffer.GetData<float, Buffer::Interop::CUDA>());
            REQUIRE(!a_buffer.GetData<uint8_t, Buffer::Interop::CUDA>());
        }
        break;
        default:
        {
        }
        break;
    }
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer None", "[buffer][none]")
{
    Context context({ {}, {}, Context::GraphicsAPI::NONE });
    Buffer& buffer = context.GetBuffer();

    REQUIRE(!buffer.GetData());
    REQUIRE(buffer.GetSize() == 0);
    REQUIRE(buffer.GetPitch() == 0);
    REQUIRE(buffer.GetWidth() == 0);
    REQUIRE(buffer.GetHeight() == 0);
    REQUIRE(buffer.GetFormat() == Buffer::Format::NONE);
    REQUIRE(buffer.GetInterop() == Buffer::Interop::NONE);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Default", "[buffer][default]")
{
    Context context({});
    RequireBufferValues(context.GetBuffer(), {});
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Invalid", "[buffer][invalid]")
{
    REQUIRE(Buffer::Config::Invalid().width == 0);
    REQUIRE(Buffer::Config::Invalid().height == 0);
    REQUIRE(Buffer::Config::Invalid().format == Buffer::Format::NONE);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Config", "[buffer][config]")
{
    Buffer::Config bufferConfig;
    Context context({ bufferConfig, {} });
    RequireBufferValues(context.GetBuffer(), bufferConfig);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Width Height", "[buffer][width][height]")
{
    constexpr uint32_t width = 800;
    constexpr uint32_t height = 600;
    Context context({ { width, height }, {} });
    RequireBufferValues(context.GetBuffer(), { width, height });
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Multiple", "[buffer][multiple]")
{
    Buffer::Config bufferConfig;
    Context context({ bufferConfig, {} });
    RequireBufferValues(context.GetBuffer(), bufferConfig);

    bufferConfig.width = 9;
    bufferConfig.height = 9;
    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    Context context2({ bufferConfig, {} });
    RequireBufferValues(context2.GetBuffer(), bufferConfig);

    {
        bufferConfig.width = 800;
        bufferConfig.height = 600;
        bufferConfig.format = Buffer::Format::RGBA_UINT16;
        Context context3({ bufferConfig, {} });
        RequireBufferValues(context3.GetBuffer(), bufferConfig);
    }
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer RGBA_FLOAT", "[buffer][rgba_float]")
{
    Buffer::Config bufferConfig;
    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    Context context({ bufferConfig, {} });
    RequireBufferValues(context.GetBuffer(), bufferConfig);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer RGBA_UINT8", "[buffer][rgba_uint8]")
{
    Buffer::Config bufferConfig;
    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    Context context({ bufferConfig, {} });
    RequireBufferValues(context.GetBuffer(), bufferConfig);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer RGBA_UINT16", "[buffer][rgba_uint16]")
{
    Buffer::Config bufferConfig;
    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    Context context({ bufferConfig, {} });
    RequireBufferValues(context.GetBuffer(), bufferConfig);
}

#ifdef CUDA_SUPPORTED
//--------------------------------------------------------------
TEST_CASE("Test Buffer Interop CUDA", "[buffer][interop][cuda]")
{
    Buffer::Config bufferConfig;
    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    bufferConfig.interop = Buffer::Interop::CUDA;

    Context context({ bufferConfig, {} });
    Buffer& buffer = context.GetBuffer();
    RequireBufferValues(buffer, bufferConfig);

    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    buffer.Resize(bufferConfig);
    RequireBufferValues(buffer, bufferConfig);

    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    buffer.Resize(bufferConfig);
    RequireBufferValues(buffer, bufferConfig);

    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    bufferConfig.interop = Buffer::Interop::HOST;
    buffer.Resize(bufferConfig);
    RequireBufferValues(buffer, bufferConfig);
}
#endif // CUDA_SUPPORTED

//--------------------------------------------------------------
TEST_CASE("Test Buffer Resize", "[buffer][resize]")
{
    Buffer::Config bufferConfig;
    Context context({ bufferConfig, {} });

    Buffer& buffer = context.GetBuffer();
    RequireBufferValues(buffer, bufferConfig);

    bufferConfig.width = 10;
    bufferConfig.height = 3;
    buffer.Resize(bufferConfig);
    RequireBufferValues(buffer, bufferConfig);

    bufferConfig.width = 99;
    bufferConfig.height = 999;
    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    buffer.Resize(bufferConfig);
    RequireBufferValues(buffer, bufferConfig);

    bufferConfig.width = 800;
    bufferConfig.height = 600;
    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    buffer.Resize(bufferConfig);
    RequireBufferValues(buffer, bufferConfig);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Min Size", "[buffer][size]")
{
    Buffer::Config bufferConfig;
    bufferConfig.width = 9;
    bufferConfig.height = 9;

    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    REQUIRE(Buffer::MinSizeBytes(bufferConfig) == 1296);

    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    REQUIRE(Buffer::MinSizeBytes(bufferConfig) == 324);

    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    REQUIRE(Buffer::MinSizeBytes(bufferConfig) == 648);

    bufferConfig.height = 100;

    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    REQUIRE(Buffer::MinSizeBytes(bufferConfig) == 14400);

    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    REQUIRE(Buffer::MinSizeBytes(bufferConfig) == 3600);

    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    REQUIRE(Buffer::MinSizeBytes(bufferConfig) == 7200);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Min Pitch", "[buffer][pitch]")
{
    Buffer::Config bufferConfig;
    bufferConfig.width = 9;
    bufferConfig.height = 9;

    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    REQUIRE(Buffer::MinPitchBytes(bufferConfig) == 144);

    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    REQUIRE(Buffer::MinPitchBytes(bufferConfig) == 36);

    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    REQUIRE(Buffer::MinPitchBytes(bufferConfig) == 72);

    bufferConfig.height = 100;

    bufferConfig.format = Buffer::Format::RGBA_FLOAT;
    REQUIRE(Buffer::MinPitchBytes(bufferConfig) == 144);

    bufferConfig.format = Buffer::Format::RGBA_UINT8;
    REQUIRE(Buffer::MinPitchBytes(bufferConfig) == 36);

    bufferConfig.format = Buffer::Format::RGBA_UINT16;
    REQUIRE(Buffer::MinPitchBytes(bufferConfig) == 72);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Bytes Per Pixel", "[buffer][bytes]")
{
    REQUIRE(Buffer::BytesPerPixel(Buffer::Format::RGBA_FLOAT) == 16);
    REQUIRE(Buffer::BytesPerPixel(Buffer::Format::RGBA_UINT8) == 4);
    REQUIRE(Buffer::BytesPerPixel(Buffer::Format::RGBA_UINT16) == 8);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Bytes Per Channel", "[buffer][bytes]")
{
    REQUIRE(Buffer::BytesPerChannel(Buffer::Format::RGBA_FLOAT) == 4);
    REQUIRE(Buffer::BytesPerChannel(Buffer::Format::RGBA_UINT8) == 1);
    REQUIRE(Buffer::BytesPerChannel(Buffer::Format::RGBA_UINT16) == 2);
}

//--------------------------------------------------------------
TEST_CASE("Test Buffer Channels Per Pixel", "[buffer][channels]")
{
    REQUIRE(Buffer::ChannelsPerPixel(Buffer::Format::RGBA_FLOAT) == 4);
    REQUIRE(Buffer::ChannelsPerPixel(Buffer::Format::RGBA_UINT8) == 4);
    REQUIRE(Buffer::ChannelsPerPixel(Buffer::Format::RGBA_UINT16) == 4);
}
