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
#include <fcntl.h>
#include <unistd.h>
}

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "atfprivate/application.hpp"
#include "atfprivate/atffile.hpp"
#include "atfprivate/exceptions.hpp"
#include "atfprivate/fs.hpp"
#include "atfprivate/io.hpp"
#include "atfprivate/serial.hpp"
#include "atfprivate/text.hpp"
#include "atfprivate/ui.hpp"

#include "atf/test_case.hpp"

class atf_run : public atf::application {
    static const char* m_description;

    size_t m_tps;
    size_t m_tcs_passed, m_tcs_failed, m_tcs_skipped;

    int run_test(const atf::fs::path&);
    int run_test_directory(const atf::fs::path&);
    int run_test_program(const atf::fs::path&);

    std::string specific_args(void) const;

public:
    atf_run(void);

    int main(void);
};

const char* atf_run::m_description =
    "atf-run is a tool that runs tests programs and collects their "
    "results.";

atf_run::atf_run(void) :
    application(m_description, "atf-run(1)")
{
}

std::string
atf_run::specific_args(void)
    const
{
    return "[test-program1 .. test-programN]";
}

int
atf_run::run_test(const atf::fs::path& tp)
{
    atf::fs::file_info fi(tp);

    int errcode;
    if (fi.get_type() == atf::fs::file_info::dir_type)
        errcode = run_test_directory(tp);
    else
        errcode = run_test_program(tp);
    return errcode;
}

int
atf_run::run_test_directory(const atf::fs::path& tp)
{
    atf::atffile af(tp / "Atffile");

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = af.begin();
         iter != af.end(); iter++)
        ok &= (run_test(tp / *iter) == EXIT_SUCCESS);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int
atf_run::run_test_program(const atf::fs::path& tp)
{
    atf::io::pipe respipe;
    pid_t pid = ::fork();
    if (pid == -1) {
        throw atf::system_error("run_test_program",
                                "fork(2) failed", errno);
    } else if (pid == 0) {
        respipe.rend().close();
        atf::io::file_handle fhres = respipe.wend().get();
        fhres.posix_remap(9);

        int nullfd = ::open("/dev/null", O_WRONLY);
        if (nullfd != -1) {
            atf::io::file_handle nullfh(nullfd);
            atf::io::file_handle nullfh2 =
                atf::io::file_handle::posix_dup(nullfd);

            ::close(STDOUT_FILENO);
            nullfh.posix_remap(STDOUT_FILENO);

            ::close(STDERR_FILENO);
            nullfh2.posix_remap(STDERR_FILENO);
        }

        std::string file = tp.leaf_name();

        // XXX Should this use -s instead?  Or do we really want to switch
        // to the target directory?
        ::chdir(tp.branch_path().c_str());

        char * args[3];
        args[0] = new char[file.length() + 1];
        std::strcpy(args[0], file.c_str());
        args[1] = new char[5];
        std::strcpy(args[1], "-r9");
        args[2] = NULL;

        ::execv(file.c_str(), args);

        std::cerr << "Failed to execute `" << tp.str() << "': "
                  << std::strerror(errno) << std::endl;
        ::exit(128);
        // TODO Account the tests that were not executed and report that
        // as an error!
    }

    respipe.wend().close();
    atf::io::file_handle fhres = respipe.rend().get();
    atf::io::pistream in(fhres);
    atf::serial::internalizer i(in, "application/X-atf-tcs", 0);

    std::cout << tp.str() << ": Running test cases" << std::endl;

    size_t passed = 0, skipped = 0, failed = 0;
    atf::tcname_tcr tt;
    while ((i >> tt).good()) {
        const std::string& tcname = tt.first;
        const atf::test_case_result& tcr = tt.second;

        std::string tag = "    " + tcname + ": ";
        std::string msg;
        if (tcr.get_status() == atf::test_case_result::status_passed) {
            passed++;
            msg = "Passed.";
        } else if (tcr.get_status() == atf::test_case_result::status_skipped) {
            skipped++;
            msg = "Skipped: " + tcr.get_reason();
        } else if (tcr.get_status() == atf::test_case_result::status_failed) {
            failed++;
            msg = "Failed: " + tcr.get_reason();
        } else {
            // XXX Bogus test.
        }

        std::cout << atf::ui::format_text_with_tag(msg, tag, false,
                                                   tag.length())
                  << std::endl;
    }

    std::cout << "    Summary: " << passed << " passed, " << skipped
              << " skipped, " << failed << " failed" << std::endl
              << std::endl;

    in.close();

    m_tps++;
    m_tcs_passed += passed;
    m_tcs_failed += failed;
    m_tcs_skipped += skipped;

    int status;
    if (::waitpid(pid, &status, 0) == -1)
        throw atf::system_error("atf_run::run_test_program(" + tp.str() +
                                ")", "waitpid(2) failed", errno);

    int code;
    if (WIFEXITED(status)) {
        code = WEXITSTATUS(status);
        if (failed > 0 && code == EXIT_SUCCESS) {
            // XXX Bogus test.
        }
    } else
        code = EXIT_FAILURE;
    return code;
}

int
atf_run::main(void)
{
    std::vector< std::string > tps;
    if (m_argc < 1) {
        tps = atf::atffile();
    } else {
        for (int i = 0; i < m_argc; i++)
            tps.push_back(m_argv[i]);
    }

    m_tps = 0;
    m_tcs_passed = m_tcs_failed = m_tcs_skipped = 0;

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = tps.begin();
         iter != tps.end(); iter++)
        ok &= (run_test(atf::fs::path(*iter)) == EXIT_SUCCESS);

    if (m_tps > 0) {
        std::cout << "Summary for " << m_tps << " executed test programs"
                  << std::endl;
        std::cout << "    " << m_tcs_passed << " test cases passed."
                  << std::endl;
        std::cout << "    " << m_tcs_failed << " test cases failed."
                  << std::endl;
        std::cout << "    " << m_tcs_skipped << " test cases were skipped."
                  << std::endl;
    } else {
        std::cout << "No tests run." << std::endl;
    }

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int
main(int argc, char* const* argv)
{
    return atf_run().run(argc, argv);
}
