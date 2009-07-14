//
// Automated Testing Framework (atf)
//
// Copyright (c) 2008, 2009 The NetBSD Foundation, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
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
#include "atf-c/error.h"
#include "atf-c/process.h"
}

#include "atf-c++/exceptions.hpp"
#include "atf-c++/process.hpp"
#include "atf-c++/sanity.hpp"

namespace impl = atf::process;
#define IMPL_NAME "atf::process"

// ------------------------------------------------------------------------
// The "stream" types.
// ------------------------------------------------------------------------

impl::basic_stream::basic_stream(void) :
    m_inited(false)
{
}

impl::basic_stream::~basic_stream(void)
{
    if (m_inited)
        atf_process_stream_fini(&m_sb);
}

const atf_process_stream_t*
impl::basic_stream::get_sb(void)
    const
{
    INV(m_inited);
    return &m_sb;
}

impl::stream_capture::stream_capture(void)
{
    atf_error_t err = atf_process_stream_init_capture(&m_sb);
    if (atf_is_error(err))
        throw_atf_error(err);
    m_inited = true;
}

impl::stream_inherit::stream_inherit(void)
{
    atf_error_t err = atf_process_stream_init_inherit(&m_sb);
    if (atf_is_error(err))
        throw_atf_error(err);
    m_inited = true;
}

impl::stream_redirect_fd::stream_redirect_fd(const int fd)
{
    atf_error_t err = atf_process_stream_init_redirect_fd(&m_sb, fd);
    if (atf_is_error(err))
        throw_atf_error(err);
    m_inited = true;
}

impl::stream_redirect_path::stream_redirect_path(const fs::path& p)
{
    atf_error_t err = atf_process_stream_init_redirect_path(&m_sb, p.c_path());
    if (atf_is_error(err))
        throw_atf_error(err);
    m_inited = true;
}

// ------------------------------------------------------------------------
// The "status" type.
// ------------------------------------------------------------------------

impl::status::status(atf_process_status_t& s) :
    m_status(s)
{
}

impl::status::~status(void)
{
    atf_process_status_fini(&m_status);
}

bool
impl::status::exited(void)
    const
{
    return atf_process_status_exited(&m_status);
}

int
impl::status::exitstatus(void)
    const
{
    return atf_process_status_exitstatus(&m_status);
}

bool
impl::status::signaled(void)
    const
{
    return atf_process_status_signaled(&m_status);
}

int
impl::status::termsig(void)
    const
{
    return atf_process_status_termsig(&m_status);
}

bool
impl::status::coredump(void)
    const
{
    return atf_process_status_coredump(&m_status);
}

// ------------------------------------------------------------------------
// The "child" type.
// ------------------------------------------------------------------------

impl::child::child(atf_process_child_t& c) :
    m_child(c)
{
}

impl::child::~child(void)
{
}

impl::status
impl::child::wait(void)
{
    atf_process_status_t s;

    atf_error_t err = atf_process_child_wait(&m_child, &s);
    if (atf_is_error(err))
        throw_atf_error(err);

    return status(s);
}

pid_t
impl::child::pid(void)
    const
{
    return atf_process_child_pid(&m_child);
}

atf::io::file_handle
impl::child::stdout_fd(void)
{
    return io::file_handle(atf_process_child_stdout(&m_child));
}

atf::io::file_handle
impl::child::stderr_fd(void)
{
    return io::file_handle(atf_process_child_stderr(&m_child));
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

void
impl::system(const std::string& cmdline)
{
    atf_error_t err;

    err = atf_process_system(cmdline.c_str());
    if (atf_is_error(err))
        throw_atf_error(err);
}
