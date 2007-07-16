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

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "atfprivate/application.hpp"
#include "atfprivate/filesystem.hpp"
#include "atfprivate/expand.hpp"
#include "atfprivate/ui.hpp"
#include "atf/test_case.hpp"

static
std::string
format_tcr(const std::string& ident,
           const atf::test_case_result& tcr)
{
    atf::test_case_result::status s = tcr.get_status();
    const std::string& r = tcr.get_reason();

    std::string str = ident + ", ";

    switch (s) {
    case atf::test_case_result::status_passed:
        str += "passed";
        break;

    case atf::test_case_result::status_skipped:
        str += "skipped, " + r;
        break;

    case atf::test_case_result::status_failed:
        str += "failed, " + r;
        break;

    default:
        assert(false);
    }

    return str;
}

class test_program : public atf::application {
public:
    typedef std::vector< atf::test_case * > test_cases;

private:
    static const char* m_description;

    bool m_lflag;
    std::string m_srcdir;
    std::string m_workdir;
    std::set< std::string > m_tcnames;

    std::string specific_args(void) const;
    options_set specific_options(void) const;
    void process_option(int, const char*);

    test_cases m_test_cases;

    test_cases init_test_cases(void);
    static test_cases filter_test_cases(test_cases,
                                        const std::set< std::string >&);

    int list_test_cases(void);
    int run_test_cases(void);

public:
    test_program(const test_cases&);

    int main(void);
};

const char* test_program::m_description =
    "This is an independent atf test program.";

test_program::test_program(const test_cases& tcs) :
    application(m_description),
    m_lflag(false),
    m_srcdir(atf::get_work_dir()),
    m_workdir(atf::get_temp_dir()),
    m_test_cases(tcs)
{
}

std::string
test_program::specific_args(void)
    const
{
    return "[test_case1 [.. test_caseN]]";
}

test_program::options_set
test_program::specific_options(void)
    const
{
    options_set opts;
    opts.insert(option('l', "", "List test cases and their purpose"));
    opts.insert(option('s', "srcdir", "Directory where the test's data "
                                      "files are located"));
    opts.insert(option('w', "workdir", "Base directory where the test cases "
                                       "will put temporary files"));
    return opts;
}

void
test_program::process_option(int ch, const char* arg)
{
    switch (ch) {
    case 'l':
        m_lflag = true;
        break;

    case 's':
        m_srcdir = arg;
        break;

    case 'w':
        m_workdir = arg;
        break;

    default:
        assert(false);
    }
}

test_program::test_cases
test_program::init_test_cases(void)
{
    test_cases tcs = m_test_cases;

    for (test_cases::iterator iter = tcs.begin();
         iter != tcs.end(); iter++) {
        atf::test_case* tc = *iter;

        tc->init();
        tc->set("srcdir", m_srcdir);
        tc->set("workdir", m_workdir);
    }

    return tcs;
}

//
// An auxiliary unary predicate that compares the given test case's
// identifier to the identifier stored in it.
//
class tc_equal_to_ident {
    const std::string& m_ident;

public:
    tc_equal_to_ident(const std::string& i) :
        m_ident(i)
    {
    }

    bool operator()(const atf::test_case* tc)
    {
        return tc->get("ident") == m_ident;
    }
};

test_program::test_cases
test_program::filter_test_cases(test_cases tcs,
                                const std::set< std::string >& tcnames)
{
    test_cases tcso;

    if (tcnames.empty()) {
        // Special case: added for efficiency because this is the most
        // typical situation.
        tcso = tcs;
    } else {
        // Collect all the test cases' identifiers.
        std::set< std::string > ids;
        for (test_cases::iterator iter = tcs.begin();
             iter != tcs.end(); iter++) {
            atf::test_case* tc = *iter;

            ids.insert(tc->get("ident"));
        }

        // Iterate over all names provided by the user and, for each one,
        // expand it as if it were a glob pattern.  Collect all expansions.
        std::set< std::string > exps;
        for (std::set< std::string >::const_iterator iter = tcnames.begin();
             iter != tcnames.end(); iter++) {
            const std::string& glob = *iter;

            std::set< std::string > ms = atf::expand_glob(glob, ids);
            if (ms.empty())
                throw std::runtime_error("Unknown test case `" + glob + "'");
            exps.insert(ms.begin(), ms.end());
        }

        // For each expansion, locate its corresponding test case and add
        // it to the output set.
        for (std::set< std::string >::const_iterator iter = exps.begin();
             iter != exps.end(); iter++) {
            const std::string& name = *iter;

            test_cases::iterator tciter =
                std::find_if(tcs.begin(), tcs.end(), tc_equal_to_ident(name));
            assert(tciter != tcs.end());
            tcso.push_back(*tciter);
        }
    }

    return tcso;
}

int
test_program::list_test_cases(void)
{
    test_cases tcs = filter_test_cases(init_test_cases(), m_tcnames);

    std::string::size_type maxlen = 0;
    for (test_cases::const_iterator iter = tcs.begin();
         iter != tcs.end(); iter++) {
        const atf::test_case* tc = *iter;

        if (maxlen < tc->get("ident").length())
            maxlen = tc->get("ident").length();
    }

    for (test_cases::const_iterator iter = tcs.begin();
         iter != tcs.end(); iter++) {
        const atf::test_case* tc = *iter;

        std::cout << atf::format_text_with_tag(tc->get("descr"),
                                               tc->get("ident"),
                                               false, maxlen + 4)
                  << std::endl;
    }

    return EXIT_SUCCESS;
}

int
test_program::run_test_cases(void)
{
    test_cases tcs = filter_test_cases(init_test_cases(), m_tcnames);

    int errcode = EXIT_SUCCESS;

    for (test_cases::iterator iter = tcs.begin();
         iter != tcs.end(); iter++) {
        atf::test_case* tc = *iter;

        atf::test_case_result tcr = tc->run();
        std::cout << format_tcr(tc->get("ident"), tcr) << std::endl;
        if (tcr.get_status() == atf::test_case_result::status_failed)
            errcode = EXIT_FAILURE;
    }

    return errcode;
}

int
test_program::main(void)
{
    int errcode;

    if (!atf::exists(m_srcdir + "/" + m_prog_name))
        throw std::runtime_error("Cannot find the test program in the "
                                 "source directory `" + m_srcdir + "'");

    if (!atf::exists(m_workdir))
        throw std::runtime_error("Cannot find the work directory `" +
                                 m_workdir + "'");

    for (int i = 0; i < m_argc; i++)
        m_tcnames.insert(m_argv[i]);

    if (m_lflag)
        errcode = list_test_cases();
    else
        errcode = run_test_cases();

    return errcode;
}

namespace atf {
    int run_test_program(int, char* const*,
                         const test_program::test_cases&);
}

int
atf::run_test_program(int argc, char* const* argv,
                      const test_program::test_cases& tcs)
{
    return test_program(tcs).run(argc, argv);
}
