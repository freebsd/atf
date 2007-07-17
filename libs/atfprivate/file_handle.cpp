//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007 The NetBSD Foundation, Inc.
// All rights reserved.
//
// This code is derived from software contributed to The NetBSD Foundation
// by Julio M. Merino Vidal, developed as part of Google's Summer of Code
// 2007 program.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this
//    software must display the following acknowledgement:
//        This product includes software developed by the NetBSD
//        Foundation, Inc. and its contributors.
// 4. Neither the name of The NetBSD Foundation nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
// CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

extern "C" {
#   include <unistd.h>
}

#include <cassert>
#include <cerrno>

#include <atfprivate/exceptions.hpp>
#include <atfprivate/file_handle.hpp>

atf::file_handle::file_handle(void) :
    m_handle(invalid_value())
{
}

atf::file_handle::file_handle(handle_type h) :
    m_handle(h)
{
    assert(m_handle != invalid_value());
}

atf::file_handle::file_handle(const file_handle& fh) :
    m_handle(fh.m_handle)
{
    fh.m_handle = invalid_value();
}

atf::file_handle::~file_handle(void)
{
    if (is_valid())
        close();
}

atf::file_handle&
atf::file_handle::operator=(const file_handle& fh)
{
    m_handle = fh.m_handle;
    fh.m_handle = invalid_value();

    return *this;
}

bool
atf::file_handle::is_valid(void)
    const
{
    return m_handle != invalid_value();
}

void
atf::file_handle::close(void)
{
    assert(is_valid());

    ::close(m_handle);

    m_handle = invalid_value();
}

atf::file_handle::handle_type
atf::file_handle::disown(void)
{
    assert(is_valid());

    handle_type h = m_handle;
    m_handle = invalid_value();
    return h;
}

atf::file_handle::handle_type
atf::file_handle::get(void)
    const
{
    assert(is_valid());

    return m_handle;
}

void
atf::file_handle::posix_remap(handle_type h)
{
    assert(is_valid());

    if (::dup2(m_handle, h) == -1)
        throw system_error("atf::file_handle::posix_remap",
                           "dup2(2) failed", errno);

    if (::close(m_handle) == -1) {
        ::close(h);
        throw system_error("atf::file_handle::posix_remap",
                           "close(2) failed", errno);
    }

    m_handle = h;
}

atf::file_handle
atf::file_handle::posix_dup(int h1)
{
    int h2 = ::dup(h1);
    if (h2 == -1)
        throw system_error("atf::file_handle::posix_dup",
                           "dup2(2) failed", errno);

    return file_handle(h2);
}

atf::file_handle
atf::file_handle::posix_dup(int h1, int h2)
{
    if (::dup2(h1, h2) == -1)
        throw system_error("atf::file_handle::posix_dup",
                           "dup2(2) failed", errno);

    return file_handle(h2);
}

const atf::file_handle::handle_type
atf::file_handle::invalid_value(void)
{
    return -1;
}
