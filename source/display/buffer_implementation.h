//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <simple/display/buffer.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{

//--------------------------------------------------------------
class Buffer::Implementation
{
public:
    // Public destructor for unique_ptr.
    virtual ~Implementation() = default;

protected:
    friend class Buffer;
    Implementation() = default;

    Implementation(const Implementation&) = delete;
    Implementation& operator=(const Implementation&) = delete;

    virtual void Resize(const Config& a_config) = 0;
    virtual void Render(uint32_t a_displayWidth,
                        uint32_t a_displayHeight) = 0;

    virtual void* GetData() const = 0;
    virtual uint32_t GetSize() const = 0;
    virtual uint32_t GetPitch() const = 0;
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual Format   GetFormat() const = 0;
};

} // namespace Display
} // namespace Simple
