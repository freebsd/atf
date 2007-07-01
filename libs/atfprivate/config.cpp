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
#include <cstdlib>
#include <map>

#include "atfprivate/config.hpp"

static std::map< std::string, std::string > m_variables;

static
void
init_variables(void)
{
    const char* str;

    assert(m_variables.empty());

    str = ::getenv("ATF_LIBEXECDIR");
    if (str != NULL)
        m_variables["atf_libexecdir"] = str;
    else
        m_variables["atf_libexecdir"] = ATF_LIBEXECDIR;

    str = ::getenv("ATF_PKGDATADIR");
    if (str != NULL)
        m_variables["atf_pkgdatadir"] = str;
    else
        m_variables["atf_pkgdatadir"] = ATF_PKGDATADIR;

    str = ::getenv("ATF_SHELL");
    if (str != NULL)
        m_variables["atf_shell"] = str;
    else
        m_variables["atf_shell"] = ATF_SHELL;

    assert(!m_variables.empty());
}

bool
atf::config::has(const std::string& varname)
{
    if (m_variables.empty())
        init_variables();

    return m_variables.find(varname) != m_variables.end();
}

const std::string&
atf::config::get(const std::string& varname)
{
    if (m_variables.empty())
        init_variables();

    assert(has(varname));
    return m_variables[varname];
}

const std::map< std::string, std::string >&
atf::config::get_all(void)
{
    if (m_variables.empty())
        init_variables();

    return m_variables;
}
