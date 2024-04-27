//--------------------------------------------------------------
// Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
//
// This code is licensed under the MIT License, a copy of which
// can be found in the license.txt file included at the root of
// this distribution, or at https://opensource.org/licenses/MIT
//--------------------------------------------------------------

#include <display/context_implementation.h>
#include <display/buffer_implementation.h>

using namespace Simple::Display;

//--------------------------------------------------------------
//! Create a display context using supplied configuration values.
//!
//! \param[in] a_config The values needed to create the context.
//--------------------------------------------------------------
Context::Context(const Config& a_config)
    : m_pimpl(Context::Implementation::Create(a_config))
{
}

//--------------------------------------------------------------
Context::~Context()
{
}

//--------------------------------------------------------------
//! Get the display buffer associated with this display context.
//!
//! \return Display buffer associated with this display context.
//--------------------------------------------------------------
Buffer& Context::GetBuffer() const
{
    static Buffer emptyBuffer(nullptr);
    return m_pimpl ? m_pimpl->GetBuffer() : emptyBuffer;
}

//--------------------------------------------------------------
//! Get the display window associated with this display context.
//!
//! \return Display window associated with this display context,
//!         or nullptr if there is no associated display window.
//--------------------------------------------------------------
Window* Context::GetWindow() const
{
    return m_pimpl ? m_pimpl->GetWindow() : nullptr;
}

//--------------------------------------------------------------
//! Call at the start of each frame to update/pump window events.
//--------------------------------------------------------------
void Context::OnFrameStart()
{
    if (m_pimpl)
    {
        m_pimpl->OnFrameStart();
    }
}

//--------------------------------------------------------------
//! Call at the end of each frame to render/display the buffer.
//--------------------------------------------------------------
void Context::OnFrameEnded()
{
    if (m_pimpl)
    {
        m_pimpl->OnFrameEnded();
    }
}
