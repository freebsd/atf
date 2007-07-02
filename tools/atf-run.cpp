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
#include <sys/stat.h>
#include <sys/wait.h>
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
#include "atfprivate/filesystem.hpp"
#include "atfprivate/pipe.hpp"
#include "atfprivate/pistream.hpp"
#include "atfprivate/ui.hpp"

class atf_run : public atf::application {
    static const char* m_description;

    size_t m_tps;
    size_t m_tcs_passed, m_tcs_failed, m_tcs_skipped;

    int run_test(const std::string&);
    int run_test_directory(const std::string&);
    int run_test_program(const std::string&);

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
atf_run::run_test(const std::string& tp)
{
    struct stat sb;

    if (::stat(tp.c_str(), &sb) == -1)
        throw atf::system_error("atf_run::run_test",
                                "stat(2) failed", errno);

    int errcode;
    if (sb.st_mode & S_IFDIR)
        errcode = run_test_directory(tp);
    else
        errcode = run_test_program(tp);
    return errcode;
}

int
atf_run::run_test_directory(const std::string& tp)
{
    atf::atffile af(tp + "/Atffile");

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = af.begin();
         iter != af.end(); iter++)
        ok &= (run_test(tp + "/" + *iter) == EXIT_SUCCESS);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int
atf_run::run_test_program(const std::string& tp)
{
    atf::pipe outpipe;
    pid_t pid = ::fork();
    if (pid == -1) {
        throw atf::system_error("run_test_program",
                                "fork(2) failed", errno);
    } else if (pid == 0) {
        outpipe.rend().close();
        atf::file_handle fhout = outpipe.wend().get();
        fhout.posix_remap(STDOUT_FILENO);

        ::execl(tp.c_str(), atf::get_leaf_name(tp).c_str(),
                NULL);

        std::cerr << "Failed to execute `" << tp << "': "
                  << std::strerror(errno) << std::endl;
        ::exit(128);
        // TODO Account the tests that were not executed and report that
        // as an error!
    }

    outpipe.wend().close();
    atf::file_handle fhout = outpipe.rend().get();
    atf::pistream in(fhout);

    std::string ident;
    {
        std::string dir = atf::get_branch_path(tp);
        if (dir == ".")
            dir = "/";
        else
            dir = "/" + dir + "/";
        ident = atf::identify(atf::get_leaf_name(tp),
                              atf::get_work_dir() + dir);
    }

    std::cout << ident << ": Running test cases" << std::endl;

    size_t passed = 0, skipped = 0, failed = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::vector< std::string > words = atf::split(line, ", ");

        const std::string& tc = words[0];
        const std::string& status = words[1];
        const std::string& reason = words.size() == 3 ? words[2] : "";

        std::string tag = "    " + tc + ": ";
        std::string msg;
        if (status == "passed") {
            passed++;
            msg = "Passed.";
        } else if (status == "skipped") {
            skipped++;
            msg = "Skipped: " + reason;
        } else if (status == "failed") {
            failed++;
            msg = "Failed: " + reason;
        } else {
            // XXX Bogus test.
        }

        std::cout << tag << atf::format_text(msg, tag.length(), tag.length())
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
        throw atf::system_error("atf_run::run_test_program",
                                "waitpid(2) failed", errno);

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
        ok &= (run_test(*iter) == EXIT_SUCCESS);

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
