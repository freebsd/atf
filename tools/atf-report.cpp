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

#include <iostream>

#include "atfprivate/application.hpp"
#include "atfprivate/serial.hpp"

#include "atf/formats.hpp"

class atf_report : public atf::application {
    static const char* m_description;

public:
    atf_report(void);

    int main(void);
};

const char* atf_report::m_description =
    "atf-report is a tool that parses the output of atf-report and "
    "generates user-friendly reports in multiple different formats.";

atf_report::atf_report(void) :
    application(m_description, "atf-report(1)")
{
}

class reader : public atf::formats::atf_tps_reader {
    size_t m_ntps;
    size_t m_tcs_passed, m_tcs_failed, m_tcs_skipped;

    void
    got_ntps(size_t ntps)
    {
        m_ntps = ntps;
        std::cout << "Running " << ntps << " test programs" << std::endl
                  << std::endl;
    }

    void
    got_tp_start(const atf::fs::path& tp, size_t ntcs)
    {
        std::cout << tp.str() << ": " << ntcs << " test cases" << std::endl;
    }

    void
    got_tp_end(void)
    {
        std::cout << std::endl;
    }

    void
    got_tc_start(const std::string& tcname)
    {
        std::cout << "    " << tcname << ": ";
        std::cout.flush();
    }

    void
    got_tc_end(const atf::tests::tcr& tcr)
    {
        atf::tests::tcr::status s = tcr.get_status();
        if (s == atf::tests::tcr::status_passed) {
            std::cout << "Passed." << std::endl;
            m_tcs_passed++;
        } else if (s == atf::tests::tcr::status_failed) {
            std::cout << "Failed: " << tcr.get_reason() << std::endl;
            m_tcs_failed++;
        } else if (s == atf::tests::tcr::status_skipped) {
            std::cout << "Skipped: " << tcr.get_reason() << std::endl;
            m_tcs_skipped++;
        }
    }

    void
    got_eof(void)
    {
        std::cout << "Summary for " << m_ntps << " test programs:"
                  << std::endl;
        std::cout << "    " << m_tcs_passed << " passed test cases."
                  << std::endl;
        std::cout << "    " << m_tcs_failed << " failed test cases."
                  << std::endl;
        std::cout << "    " << m_tcs_skipped << " skipped test cases."
                  << std::endl;
    }

public:
    reader(std::istream& is) :
        atf::formats::atf_tps_reader(is),
        m_ntps(0),
        m_tcs_passed(0),
        m_tcs_failed(0),
        m_tcs_skipped(0)
    {
    }
};

int
atf_report::main(void)
{
    reader(std::cin).read();

    return EXIT_SUCCESS;
}

int
main(int argc, char* const* argv)
{
    return atf_report().run(argc, argv);
}
