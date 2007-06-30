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

static int run_test(const std::string& progname);

static
int
run_test_program(const std::string& progname)
{
    pid_t pid;

    atf::pipe outpipe;

    int code = EXIT_SUCCESS;

    pid = ::fork();
    if (pid == -1) {
        throw atf::system_error("run_test_program",
                                "fork(2) failed", errno);
    } else if (pid == 0) {
        outpipe.rend().close();
        atf::file_handle fhout = outpipe.wend().get();
        fhout.posix_remap(STDOUT_FILENO);

        ::execl(progname.c_str(), atf::get_leaf_name(progname).c_str(),
                NULL);

        std::cerr << "Failed to execute `" << progname << "': "
                  << std::strerror(errno) << std::endl;
        ::exit(128);
    } else {
        outpipe.wend().close();
        atf::file_handle fhout = outpipe.rend().get();
        atf::pistream in(fhout);

        std::string line;
        while (std::getline(in, line))
            std::cout << line << std::endl;

        in.close();

        int status;
        if (::waitpid(pid, &status, 0) == -1)
            throw atf::system_error("run_test_program",
                                    "waitpid(2) failed", errno);
        if (WIFEXITED(status))
            code = WEXITSTATUS(status);
        else
            code = EXIT_FAILURE;
    }

    return code;
}

static
int
run_test_directory(const std::string& progname)
{
    atf::atffile af(progname + "/Atffile");

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = af.begin();
         iter != af.end(); iter++)
        ok &= (run_test(progname + "/" + *iter) == EXIT_SUCCESS);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

static
int
run_test(const std::string& progname)
{
    struct stat sb;

    if (::stat(progname.c_str(), &sb) == -1)
        throw atf::system_error("run_test",
                                "stat(2) failed", errno);

    int errcode;
    if (sb.st_mode & S_IFDIR)
        errcode = run_test_directory(progname.c_str());
    else
        errcode = run_test_program(progname.c_str());
    return errcode;
}

class atf_run : public atf::application {
    static const char* m_description;

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
atf_run::main(void)
{
    std::vector< std::string > tps;
    if (m_argc < 1) {
        tps = atf::atffile();
    } else {
        for (int i = 0; i < m_argc; i++)
            tps.push_back(m_argv[i]);
    }

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = tps.begin();
         iter != tps.end(); iter++)
        ok &= (run_test(*iter) == EXIT_SUCCESS);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int
main(int argc, char* const* argv)
{
    return atf_run().run(argc, argv);
}
