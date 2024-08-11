//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <display/buffer_implementation.h>

using namespace Simple::Display;

//--------------------------------------------------------------
Buffer::Config Buffer::Config::Invalid()
{
    return { 0, 0, Format::NONE, Interop::NONE };
}

//--------------------------------------------------------------
Buffer::Buffer(std::unique_ptr<Implementation> a_pimpl)
    : m_pimpl(std::move(a_pimpl))
{
}

//--------------------------------------------------------------
Buffer::~Buffer()
{
}

//--------------------------------------------------------------
//! Resize and/or reformat the buffer using configuration values.
//!
//! \param[in] a_config The new size/format configuration values.
//--------------------------------------------------------------
void Buffer::Resize(const Config& a_config)
{
    if (m_pimpl)
    {
        m_pimpl->Resize(a_config);
    }
}

//--------------------------------------------------------------
//! Render the buffer to the display device. Should usually only
//! be called by the Context which created this buffer instance.
//!
//! \param[in] a_displayWidth The pixel width of the display.
//! \param[in] a_displayHeight The pixel height of the display.
//--------------------------------------------------------------
void Buffer::Render(uint32_t a_displayWidth,
                    uint32_t a_displayHeight)
{
    if (m_pimpl)
    {
        m_pimpl->Render(a_displayWidth, a_displayHeight);
    }
}

//--------------------------------------------------------------
//! Get the buffer data as a host accessible array of floats.
//!
//! \return Buffer data as a host accessible array of floats,
//!         or nullptr if it cannot be accessed/cast as such.
//--------------------------------------------------------------
template<>
float* Buffer::GetData<float, Buffer::Interop::HOST>() const
{
    return (GetInterop() == Interop::HOST &&
            GetFormat() == Format::RGBA_FLOAT) ?
            static_cast<float*>(GetData()) : nullptr;
}

//--------------------------------------------------------------
//! Get the buffer data as a CUDA accessible array of floats.
//!
//! \return Buffer data as a CUDA accessible array of floats,
//!         or nullptr if it cannot be accessed/cast as such.
//--------------------------------------------------------------
template<>
float* Buffer::GetData<float, Buffer::Interop::CUDA>() const
{
    return (GetInterop() == Interop::CUDA &&
            GetFormat() == Format::RGBA_FLOAT) ?
            static_cast<float*>(GetData()) : nullptr;
}

//--------------------------------------------------------------
//! Get the buffer data as a host accessible array of uint8s.
//!
//! \return Buffer data as a host accessible array of uint8s,
//!         or nullptr if it cannot be accessed/cast as such.
//--------------------------------------------------------------
template<>
uint8_t* Buffer::GetData<uint8_t, Buffer::Interop::HOST>() const
{
    return (GetInterop() == Interop::HOST &&
            GetFormat() == Format::RGBA_UINT8) ?
            static_cast<uint8_t*>(GetData()) : nullptr;
}

//--------------------------------------------------------------
//! Get the buffer data as a CUDA accessible array of uint8s.
//!
//! \return Buffer data as a CUDA accessible array of uint8s,
//!         or nullptr if it cannot be accessed/cast as such.
//--------------------------------------------------------------
template<>
uint8_t* Buffer::GetData<uint8_t, Buffer::Interop::CUDA>() const
{
    return (GetInterop() == Interop::CUDA &&
            GetFormat() == Format::RGBA_UINT8) ?
            static_cast<uint8_t*>(GetData()) : nullptr;
}

//--------------------------------------------------------------
//! Get the buffer data as a host accessible array of uint16s.
//!
//! \return Buffer data as a host accessible array of uint16s,
//!         or nullptr if it cannot be accessed/cast as such.
//--------------------------------------------------------------
template<>
uint16_t* Buffer::GetData<uint16_t, Buffer::Interop::HOST>() const
{
    return (GetInterop() == Interop::HOST &&
            GetFormat() == Format::RGBA_UINT16) ?
            static_cast<uint16_t*>(GetData()) : nullptr;
}

//--------------------------------------------------------------
//! Get the buffer data as a CUDA accessible array of uint16s.
//!
//! \return Buffer data as a CUDA accessible array of uint16s,
//!         or nullptr if it cannot be accessed/cast as such.
//--------------------------------------------------------------
template<>
uint16_t* Buffer::GetData<uint16_t, Buffer::Interop::CUDA>() const
{
    return (GetInterop() == Interop::CUDA &&
            GetFormat() == Format::RGBA_UINT16) ?
            static_cast<uint16_t*>(GetData()) : nullptr;
}

//--------------------------------------------------------------
//! Get the raw buffer data. Should not be cached/stored between
//! frames, as the pointer address could be swapped or recreated.
//! Prefer accessing with the various GetData<> template methods.
//!
//! \return Buffer of the raw pixel data which will be displayed.
//--------------------------------------------------------------
void* Buffer::GetData() const
{
    return m_pimpl ? m_pimpl->GetData() : nullptr;
}

//--------------------------------------------------------------
//! Get the actual size (measured in bytes) of the buffer data.
//! This may be greater than the result of Buffer::MinSizeBytes
//! due to any implementation specific alignent and/or padding.
//!
//! \return Actual size (measured in bytes) of the buffer data.
//--------------------------------------------------------------
uint32_t Buffer::GetSize() const
{
    return m_pimpl ? m_pimpl->GetSize() : 0;
}

//--------------------------------------------------------------
//! Get the actual pitch (measured in bytes) of the buffer data.
//! This may be greater than the result of Buffer::MinPitchBytes
//! due to any implementation specific alignent and/or padding.
//!
//! \return Actual pitch (measured in bytes) of the buffer data.
//--------------------------------------------------------------
uint32_t Buffer::GetPitch() const
{
    return m_pimpl ? m_pimpl->GetPitch() : 0;
}

//--------------------------------------------------------------
//! Get the width of the display buffer, measured in pixels.
//!
//! \return Width of the display buffer, measured in pixels.
//--------------------------------------------------------------
uint32_t Buffer::GetWidth() const
{
    return m_pimpl ? m_pimpl->GetWidth() : 0;
}

//--------------------------------------------------------------
//! Get the height of the display buffer, measured in pixels.
//!
//! \return Height of the display buffer, measured in pixels.
//--------------------------------------------------------------
uint32_t Buffer::GetHeight() const
{
    return m_pimpl ? m_pimpl->GetHeight() : 0;
}

//--------------------------------------------------------------
//! Get the format of each pixel contained by the display buffer.
//!
//! \return Format of each pixel contained by the display buffer.
//--------------------------------------------------------------
Buffer::Format Buffer::GetFormat() const
{
    return m_pimpl ? m_pimpl->GetFormat() : Format::NONE;
}

//--------------------------------------------------------------
//! Get the type of interop used to map the display buffer data.
//!
//! \return Type of interop used to map the display buffer data.
//--------------------------------------------------------------
Buffer::Interop Buffer::GetInterop() const
{
    return m_pimpl ? m_pimpl->GetInterop() : Interop::NONE;
}
