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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
}

#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "atfprivate/env.hpp"
#include "atfprivate/exceptions.hpp"
#include "atfprivate/fs.hpp"
#include "atfprivate/text.hpp"
#include "atfprivate/user.hpp"

#include "atf/exceptions.hpp"
#include "atf/test_case.hpp"

atf::test_case::test_case(const std::string& ident) :
    m_ident(ident)
{
}

atf::test_case::~test_case(void)
{
}

void
atf::test_case::ensure_boolean(const std::string& name)
{
    ensure_not_empty(name);

    std::string val = get(name);
    for (std::string::size_type i = 0; i < val.length(); i++)
        val[i] = std::tolower(val[i]);

    if (val == "yes" || val == "true")
        set(name, "true");
    else if (val == "no" || val == "false")
        set(name, "false");
    else
        throw std::runtime_error("Invalid value for boolean variable `" +
                                 name + "'");
}

void
atf::test_case::ensure_not_empty(const std::string& name)
{
    if (m_meta_data.find(name) == m_meta_data.end())
        throw atf::not_found_error< std::string >
            ("Undefined or empty variable", name);

    variables_map::const_iterator iter = m_meta_data.find(name);
    assert(iter != m_meta_data.end());

    const std::string& val = (*iter).second;
    if (val.empty())
        throw atf::not_found_error< std::string > // XXX Incorrect error
            ("Undefined or empty variable", name);
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

    if (val == "true")
        return true;
    else if (val == "false")
        return false;
    else {
        assert(false);
        return false;
    }
}

bool
atf::test_case::has(const std::string& var)
    const
{
    variables_map::const_iterator iter = m_meta_data.find(var);
    return (iter != m_meta_data.end());
}

const std::string&
atf::test_case::get_srcdir(void)
    const
{
    return m_srcdir;
}

void
atf::test_case::init(const std::string& srcdir,
                     const std::string& workdirbase)
{
    assert(m_meta_data.empty());

    m_srcdir = srcdir;
    m_workdirbase = workdirbase;

    m_meta_data["ident"] = m_ident;
#include "tchead.cpp"
    assert(m_meta_data["ident"] == m_ident);
}

static
void
enter_workdir(const atf::test_case* tc, atf::fs::path& olddir,
              atf::fs::path& workdir, const std::string& base)
{
    if (!tc->get_bool("isolated"))
        return;

    atf::fs::path pattern = atf::fs::path(base) / "atf.XXXXXX";
    workdir = atf::fs::create_temp_dir(pattern);
    olddir = atf::fs::change_directory(workdir);
}

static
void
leave_workdir(const atf::test_case* tc, atf::fs::path& olddir,
              atf::fs::path& workdir)
{
    if (!tc->get_bool("isolated"))
        return;

    atf::fs::change_directory(olddir);
    atf::fs::cleanup(workdir);
}

atf::test_case_result
atf::test_case::safe_run(void)
    const
{
    test_case_result tcr = test_case_result::passed();

    if (has("require.user")) {
        const std::string& u = get("require.user");
        if (u == "root") {
            if (!user::is_root())
                tcr = test_case_result::skipped("Requires root privileges");
        } else if (u == "unprivileged") {
            if (!user::is_unprivileged())
                tcr = test_case_result::skipped("Requires an unprivileged "
                                                "user");
        } else
            tcr = test_case_result::skipped("Invalid value in the "
                                            "require.user property");
    }

    if (tcr.get_status() == test_case_result::status_passed) {
        // Previous checks have not made the test fail, so run it now.

        fs::path olddir(".");
        fs::path workdir(".");

        try {
            enter_workdir(this, olddir, workdir, m_workdirbase);
            tcr = fork_body(workdir.str());
            leave_workdir(this, olddir, workdir);
        } catch (...) {
            leave_workdir(this, olddir, workdir);
            throw;
        }
    }

    return tcr;
}

atf::test_case_result
atf::test_case::fork_body(const std::string& workdir)
    const
{
    test_case_result tcr;

    fs::path result(".");
    if (get_bool("isolated")) {
        result = fs::path(workdir) / "tc-result";
    } else {
        result = fs::create_temp_file(fs::path(m_workdirbase) / "atf.XXXXXX");
    }

    pid_t pid = ::fork();
    if (pid == -1) {
        tcr = test_case_result::failed("Coult not fork to run test case");
    } else if (pid == 0) {
        // Unexpected errors detected in the child process are mentioned
        // in stderr to give the user a chance to see what happened.
        // This is specially useful in those cases where these errors
        // cannot be directed to the parent process.
        try {
#include "tcenv.cpp"

            // This is here because require_progs can assert if the user
            // gave bad input.  Having it here lets us capture the abort
            // from the parent.
            //
            // XXX The thing is... is this appropriate?  Should we forbid
            // the assertions in our code and use exceptions instead?
            // Or... should all properties be handled here -- aka, in the
            // controlled environment?
            if (has("require.progs")) {
                std::vector< std::string > progs =
                    text::split(get("require.progs"), " ");
                for (std::vector< std::string >::const_iterator iter =
                     progs.begin(); iter != progs.end(); iter++)
                    require_prog(*iter);
            }

            body();
            ATF_PASS();
        } catch (const atf::test_case_result& tcre) {
            std::ofstream os(result.c_str());
            if (!os) {
                std::cerr << "Could not open the temporary file " +
                             result.str() + " to leave the results in it"
                          << std::endl;
                ::exit(EXIT_FAILURE);
            }
            os << tcre;
            os.close();
        } catch (const std::runtime_error& e) {
            std::cerr << "Caught unexpected error: " << e.what() << std::endl;
            ::exit(EXIT_FAILURE);
        } catch (...) {
            std::cerr << "Caught unexpected error" << std::endl;
            ::exit(EXIT_FAILURE);
        }
        ::exit(EXIT_SUCCESS);
    } else {
        int status;
        if (::waitpid(pid, &status, 0) == -1)
            tcr = test_case_result::failed("waitpid failed");
        else {
            std::ifstream is(result.c_str());
            if (!is) {
                tcr = test_case_result::failed("Could not open results file "
                                               "for reading");
            } else {
                try {
                    is >> tcr;
                } catch (const format_error& e) {
                    tcr = test_case_result::failed(e.what());
                }
                is.close();
            }
        }
    }

    return tcr;
}

atf::test_case_result
atf::test_case::run(void)
    const
{
    assert(!m_meta_data.empty());

    test_case_result tcr;

    try {
        tcr = safe_run();
    } catch (const std::exception& e) {
        tcr = test_case_result::failed(std::string("Unhandled "
                                                   "exception: ") +
                                       e.what());
    } catch (...) {
        tcr = test_case_result::failed("Unknown unhandled exception");
    }

    return tcr;
}

void
atf::test_case::require_prog(const std::string& prog)
    const
{
    assert(!prog.empty());

    fs::path p(prog);

    if (p.is_absolute()) {
        if (!fs::is_executable(p))
            ATF_SKIP("The required program " + prog + " could not "
                     "be found");
    } else {
        assert(p.branch_path() == fs::path("."));
        if (fs::find_prog_in_path(prog) == fs::path("."))
            ATF_SKIP("The required program " + prog + " could not "
                     "be found in the PATH");
    }
}


std::ostream&
operator<<(std::ostream& os, const atf::tcname_tcr& tt)
{
    const std::string& tcname = tt.first;
    const atf::test_case_result& tcr = tt.second;

    os << tcname << ", " << tcr;

    return os;
}

std::istream&
operator>>(std::istream& is, atf::tcname_tcr& tt)
{
    std::string tcname;
    atf::test_case_result tcr;

    std::getline(is, tcname, ',');
    if (!is.good())
        return is;
    int ch = is.get();
    if (!is.good())
        throw atf::format_error("Unexpected end of stream");
    if (ch != ' ')
        throw atf::format_error("Incorrect input separator after test"
                                "case name");
    is >> tcr;

    tt.first = tcname;
    tt.second = tcr;

    return is;
}
