//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007, 2008, 2009 The NetBSD Foundation, Inc.
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

#include <cstring>

extern "C" {
#include "atf-c/error.h"
}

#include "atf-c++/check.hpp"
#include "atf-c++/exceptions.hpp"
#include "atf-c++/sanity.hpp"

namespace impl = atf::check;
#define IMPL_NAME "atf::check"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

template< class C >
atf::utils::auto_array< const char* >
collection_to_argv(const C& c)
{
    atf::utils::auto_array< const char* > argv(new const char*[c.size() + 1]);

    std::size_t pos = 0;
    for (typename C::const_iterator iter = c.begin(); iter != c.end();
         iter++) {
        argv[pos] = (*iter).c_str();
        pos++;
    }
    INV(pos == c.size());
    argv[pos] = NULL;

    return argv;
}

template< class C >
C
argv_to_collection(const char* const* argv)
{
    C c;

    for (const char* const* iter = argv; *iter != NULL; iter++)
        c.push_back(std::string(*iter));

    return c;
}

// ------------------------------------------------------------------------
// The "check_result" class.
// ------------------------------------------------------------------------

impl::check_result::check_result(const atf_check_result_t* result)
{
    std::memcpy(&m_result, result, sizeof(m_result));

    atf_list_citer_t iter;
    atf_list_for_each_c(iter, atf_check_result_argv(&m_result))
        m_argv.push_back((const char *)atf_list_citer_data(iter));
}

impl::check_result::~check_result(void)
{
    atf_check_result_fini(&m_result);
}

const std::vector< std::string >&
impl::check_result::argv(void)
    const
{
    return m_argv;
}

bool
impl::check_result::exited(void)
    const
{
    return atf_check_result_exited(&m_result);
}

int
impl::check_result::exitcode(void)
    const
{
    PRE(exited());
    return atf_check_result_exitcode(&m_result);
}

const atf::fs::path
impl::check_result::stdout_path(void)
    const
{
    return atf_check_result_stdout(&m_result);
}

const atf::fs::path
impl::check_result::stderr_path(void)
        const
{
    return atf_check_result_stderr(&m_result);
}

// ------------------------------------------------------------------------
// The "argv_array" type.
// ------------------------------------------------------------------------

impl::argv_array::argv_array(void) :
    m_exec_argv(collection_to_argv(m_args))
{
}

impl::argv_array::argv_array(const char* const* ca) :
    m_args(argv_to_collection< args_vector >(ca)),
    m_exec_argv(collection_to_argv(m_args))
{
}

impl::argv_array::argv_array(const argv_array& a) :
    m_args(a.m_args),
    m_exec_argv(collection_to_argv(m_args))
{
}

void
impl::argv_array::ctor_init_exec_argv(void)
{
    m_exec_argv = collection_to_argv(m_args);
}

const char* const*
impl::argv_array::exec_argv(void)
    const
{
    return m_exec_argv.get();
}

impl::argv_array::size_type
impl::argv_array::size(void)
    const
{
    return m_args.size();
}

const char*
impl::argv_array::operator[](int idx)
    const
{
    return m_args[idx].c_str();
}

impl::argv_array::const_iterator
impl::argv_array::begin(void)
    const
{
    return m_args.begin();
}

impl::argv_array::const_iterator
impl::argv_array::end(void)
    const
{
    return m_args.end();
}

impl::argv_array&
impl::argv_array::operator=(const argv_array& a)
{
    if (this != &a) {
        m_args = a.m_args;
        m_exec_argv = collection_to_argv(m_args);
    }
    return *this;
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

impl::check_result
impl::exec(const argv_array& argva)
{
    atf_check_result_t result;

    atf_error_t err = atf_check_exec_array(argva.exec_argv(), &result);
    if (atf_is_error(err))
        throw_atf_error(err);

    return impl::check_result(&result);
}
