//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <memory>

//! @file

//--------------------------------------------------------------
//! The default width of any display buffer, measured in pixels.
//--------------------------------------------------------------
#ifndef DEFAULT_BUFFER_WIDTH
#define DEFAULT_BUFFER_WIDTH 1920
#endif//DEFAULT_BUFFER_WIDTH

//--------------------------------------------------------------
//! The default height of any display buffer, measured in pixels.
//--------------------------------------------------------------
#ifndef DEFAULT_BUFFER_HEIGHT
#define DEFAULT_BUFFER_HEIGHT 1080
#endif//DEFAULT_BUFFER_HEIGHT

//--------------------------------------------------------------
//! The default pixel format of any display buffer.
//--------------------------------------------------------------
#ifndef DEFAULT_BUFFER_FORMAT
#define DEFAULT_BUFFER_FORMAT Format::RGBA_UINT8
#endif//DEFAULT_BUFFER_FORMAT

//--------------------------------------------------------------
//! The default memory interop of any display buffer.
//--------------------------------------------------------------
#ifndef DEFAULT_BUFFER_INTEROP
#define DEFAULT_BUFFER_INTEROP Interop::HOST
#endif//DEFAULT_BUFFER_INTEROP

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{

//--------------------------------------------------------------
//! Class that represents a pixel buffer which can be displayed.
//!
//! The Simple::Display::Buffer class encapsulates functionality
//! for creating, resizing, and rendering to a pixel buffer that
//! can then be presented to a display device/window each frame.
//!
//! Buffers are created indirectly via Simple::Display::Context.
//--------------------------------------------------------------
class Buffer
{
public:
    class Implementation;

    //----------------------------------------------------------
    //! The format of each pixel contained by the display buffer.
    //----------------------------------------------------------
    enum class Format
    {
        NONE = 0,   //!< None/unknown/invalid pixel components.
        RGBA_FLOAT, //!< Red/green/blue/alpha float components.
        RGBA_UINT8, //!< Red/green/blue/alpha uint8 components.
        RGBA_UINT16 //!< Red/green/blue/alpha uint16 components.
    };

    //----------------------------------------------------------
    //! The type of interop used to map the display buffer data
    //! and make it accessible for writing from the application.
    //----------------------------------------------------------
    enum class Interop
    {
        NONE = 0,   //!< None/unknown/invalid memory interop.
        HOST,       //!< Data mapped to host process memory.
        CUDA        //!< Data mapped to CUDA device memory.
    };

    //----------------------------------------------------------
    //! Values needed to define Simple::Display::Buffer objects.
    //----------------------------------------------------------
    struct Config
    {
        //! The width of the display buffer, measured in pixels.
        uint32_t width = DEFAULT_BUFFER_WIDTH;

        //! The height of the display buffer, measured in pixels.
        uint32_t height = DEFAULT_BUFFER_HEIGHT;

        //! The pixel format describing the display buffer type.
        Format   format = DEFAULT_BUFFER_FORMAT;

        //! The type of interop used to map the display buffer.
        Interop  interop = DEFAULT_BUFFER_INTEROP;

        //! An invalid buffer configuration.
        static Config Invalid();
    };

    Buffer(std::unique_ptr<Implementation> a_pimpl);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    void Resize(const Config& a_config);
    void Render(uint32_t a_displayWidth,
                uint32_t a_displayHeight);

    template<typename Type, Interop InteropType = Interop::HOST>
    Type* GetData() const;

    void* GetData() const;
    uint32_t GetSize() const;
    uint32_t GetPitch() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    Format   GetFormat() const;
    Interop  GetInterop() const;

    static constexpr uint32_t MinSizeBytes(const Config&);
    static constexpr uint32_t MinPitchBytes(const Config&);
    static constexpr uint32_t BytesPerPixel(const Format&);
    static constexpr uint32_t BytesPerChannel(const Format&);
    static constexpr uint32_t ChannelsPerPixel(const Format&);

private:
    const std::unique_ptr<Implementation> m_pimpl;
};

//--------------------------------------------------------------
//! Calculate the min size in bytes required to store a buffer.
//!
//! \param[in] a_config The configuration values of the buffer.
//! \return The min size in bytes required to store the buffer.
//--------------------------------------------------------------
constexpr uint32_t Buffer::MinSizeBytes(const Config& a_config)
{
    return a_config.height * MinPitchBytes(a_config);
}

//--------------------------------------------------------------
//! Calculate the min pitch in bytes required to store a buffer.
//!
//! Also known as stride, this is the distance in bytes between
//! the starting memory addresses of consecutive rows of pixels.
//!
//! \param[in] a_config The configuration values for the buffer.
//! \return The min pitch in bytes required to store the buffer.
//--------------------------------------------------------------
constexpr uint32_t Buffer::MinPitchBytes(const Config& a_config)
{
    return a_config.width * BytesPerPixel(a_config.format);
}

//--------------------------------------------------------------
//! Calculate the number of bytes required to store a pixel.
//!
//! \param[in] a_format The format that describes the pixel.
//! \return The number of bytes required to store the pixel.
//--------------------------------------------------------------
constexpr uint32_t Buffer::BytesPerPixel(const Format& a_format)
{
    return BytesPerChannel(a_format) * ChannelsPerPixel(a_format);
}

//--------------------------------------------------------------
//! Get the number of bytes needed to store a pixel channel.
//!
//! \param[in] a_format The format that describes the pixel.
//! \return Number of bytes needed to store a pixel channel.
//--------------------------------------------------------------
constexpr uint32_t Buffer::BytesPerChannel(const Format& a_format)
{
    switch (a_format)
    {
        case Format::RGBA_FLOAT: return 4;
        case Format::RGBA_UINT8: return 1;
        case Format::RGBA_UINT16: return 2;
        default: return 0;
    }
}

//--------------------------------------------------------------
//! Get the number of channels contained by a single pixel.
//!
//! \param[in] a_format The format that describes the pixel.
//! \return The number of channels that constitute the pixel.
//--------------------------------------------------------------
constexpr uint32_t Buffer::ChannelsPerPixel(const Format& a_format)
{
    switch (a_format)
    {
        case Format::RGBA_FLOAT: return 4;
        case Format::RGBA_UINT8: return 4;
        case Format::RGBA_UINT16: return 4;
        default: return 0;
    }
}

} // namespace Display
} // namespace Simple
