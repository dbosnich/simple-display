//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

#include <simple/display/context.h>

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{

//--------------------------------------------------------------
class Context::Implementation
{
public:
    // Public destructor for unique_ptr.
    virtual ~Implementation() = default;

protected:
    // Platform specific factory function creates implementation.
    static std::unique_ptr<Implementation> Create(const Config&);

    friend class Context;
    Implementation() = default;

    Implementation(const Implementation&) = delete;
    Implementation& operator=(const Implementation&) = delete;

    virtual Buffer& GetBuffer() const = 0;
    virtual Window* GetWindow() const = 0;

    virtual void OnFrameStart() = 0;
    virtual void OnFrameEnded() = 0;
};

} // namespace Display
} // namespace Simple
