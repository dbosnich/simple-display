//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#pragma once

//--------------------------------------------------------------
namespace Simple
{
namespace Display
{
namespace OpenGL
{

//--------------------------------------------------------------
class InteropGL
{
public:
    virtual ~InteropGL() = default;
    virtual void Map(void** a_bufferData) = 0;
    virtual void Unmap() = 0;
};

} // namespace OpenGL
} // namespace Display
} // namespace Simple
