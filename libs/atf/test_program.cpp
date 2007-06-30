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
#include <iostream>
#include <vector>

#include "atfprivate/application.hpp"
#include "atfprivate/ui.hpp"
#include "atf/report.hpp"
#include "atf/test_case.hpp"

class test_program : public atf::application {
public:
    typedef std::vector< atf::test_case * > test_cases;

private:
    static const char* m_description;

    bool m_lflag;
    std::set< std::string > m_tcnames;

    std::string specific_args(void) const;
    options_set specific_options(void) const;
    void process_option(int, const char*);

    test_cases m_test_cases;

    test_cases init_test_cases(void);
    static test_cases filter_test_cases(test_cases, std::set< std::string >&);

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
    return opts;
}

void
test_program::process_option(int ch, const char* arg)
{
    switch (ch) {
    case 'l':
        m_lflag = true;
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
    }

    return tcs;
}

test_program::test_cases
test_program::filter_test_cases(test_cases tcs,
                                std::set< std::string >& tcnames)
{
    test_cases tcso;

    if (tcnames.empty())
        tcso = tcs;
    else {
        for (test_cases::iterator iter = tcs.begin();
             iter != tcs.end(); iter++) {
            atf::test_case* tc = *iter;

            if (tcnames.find(tc->get("ident")) != tcnames.end())
                tcso.push_back(tc);
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

        std::cout << tc->get("ident") << "    "
                  << atf::format_text(tc->get("descr"), maxlen + 4,
                                      tc->get("ident").length() + 4)
                  << std::endl;
    }

    return EXIT_SUCCESS;
}

int
test_program::run_test_cases(void)
{
    test_cases tcs = filter_test_cases(init_test_cases(), m_tcnames);

    int errcode = EXIT_SUCCESS;

    atf::report r(std::cout);
    for (test_cases::iterator iter = tcs.begin();
         iter != tcs.end(); iter++) {
        atf::test_case* tc = *iter;

        atf::test_case_result tcr = tc->run();
        r.log(tc->get("ident"), tcr);
        if (tcr.get_status() == atf::test_case_result::status_failed)
            errcode = EXIT_FAILURE;
    }

    return errcode;
}

int
test_program::main(void)
{
    int errcode;

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
