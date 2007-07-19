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

#include <cassert>
#include <cctype>
#include <stdexcept>

#include "atfprivate/exceptions.hpp"
#include "atfprivate/fs.hpp"
#include "atf/test_case.hpp"

atf::test_case::test_case(const std::string& ident) :
    m_ident(ident)
{
}

atf::test_case::~test_case(void)
{
}

void
atf::test_case::ensure_defined(const std::string& name)
{
    if (m_meta_data.find(name) == m_meta_data.end())
        throw atf::not_found_error< std::string >
            ("Undefined variable in test case", name);
}

void
atf::test_case::ensure_not_empty(const std::string& name)
{
    ensure_defined(name);

    variables_map::const_iterator iter = m_meta_data.find(name);
    assert(iter != m_meta_data.end());

    const std::string& val = (*iter).second;
    if (val.empty())
        throw atf::not_found_error< std::string > // XXX Incorrect error
            ("Variable empty", name);
}

void
atf::test_case::set(const std::string& var, const std::string& val)
{
    m_meta_data[var] = val;
}

const std::string&
atf::test_case::get(const std::string& var)
    const
{
    variables_map::const_iterator iter = m_meta_data.find(var);
    assert(iter != m_meta_data.end());
    return (*iter).second;
}

bool
atf::test_case::get_bool(const std::string& var)
    const
{
    std::string val = get(var);
    for (std::string::size_type i = 0; i < val.length(); i++)
        val[i] = std::tolower(val[i]);

    if (val == "yes" || val == "true")
        return true;
    else if (val == "no" || val == "false")
        return false;
    else
        throw std::runtime_error("Invalid value for boolean varabile " +
                                 var);
}

void
atf::test_case::init(void)
{
    assert(m_meta_data.empty());

    m_meta_data["isolated"] = "yes";

    head();

    assert(m_meta_data.find("ident") == m_meta_data.end());
    m_meta_data["ident"] = m_ident;

    ensure_not_empty("ident");
    ensure_not_empty("descr");
}
#include <iostream>
static
void
enter_workdir(const atf::test_case* tc, std::string& olddir,
              std::string& workdir)
{
    if (!tc->get_bool("isolated"))
        return;

    olddir = atf::get_work_dir();
    const std::string pattern = tc->get("workdir") + "/atf.XXXXXX";
    workdir = atf::create_temp_dir(pattern);
    atf::change_directory(workdir);
}

static
void
leave_workdir(const atf::test_case* tc, std::string& olddir,
              std::string& workdir)
{
    if (!tc->get_bool("isolated"))
        return;

    atf::change_directory(olddir);
    atf::rm_rf(workdir);
}

atf::test_case_result
atf::test_case::run(void)
    const
{
    assert(!m_meta_data.empty());

    test_case_result tcr = test_case_result::passed();

    std::string olddir;
    std::string workdir;

    try {
        try {
            try {
                enter_workdir(this, olddir, workdir);
                body();
                leave_workdir(this, olddir, workdir);
                assert(tcr.get_status() == test_case_result::status_passed);
            } catch (...) {
                leave_workdir(this, olddir, workdir);
                throw;
            }
        } catch (const test_case_result& tcre) {
            throw tcre;
        } catch (const std::exception& e) {
            throw test_case_result::failed(std::string("Unhandled "
                                                       "exception: ") +
                                           e.what());
        } catch (...) {
            throw test_case_result::failed("Unknown unhandled exception");
        }
    } catch (const test_case_result& tcre) {
        tcr = tcre;
    } catch (...) {
        assert(false);
    }

    return tcr;
}
