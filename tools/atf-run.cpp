//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007 The NetBSD Foundation, Inc.
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
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>

#include "atf/application.hpp"
#include "atf/atffile.hpp"
#include "atf/exceptions.hpp"
#include "atf/formats.hpp"
#include "atf/fs.hpp"
#include "atf/io.hpp"
#include "atf/tests.hpp"

class muxer : public atf::formats::atf_tcs_reader {
    atf::fs::path m_tp;
    atf::formats::atf_tps_writer m_writer;

    size_t m_ntcs;
    std::string m_tcname;

    // Counters for the test cases run by the test program.
    size_t m_passed, m_failed, m_skipped;

    void
    got_ntcs(size_t ntcs)
    {
        m_writer.start_tp(m_tp, ntcs);
    }

    void
    got_tc_start(const std::string& tcname)
    {
        m_tcname = tcname;
        m_writer.start_tc(tcname);
    }

    void
    got_tc_end(const atf::tests::tcr& tcr)
    {
        const atf::tests::tcr::status& s = tcr.get_status();
        if (s == atf::tests::tcr::status_passed) {
            m_passed++;
        } else if (s == atf::tests::tcr::status_skipped) {
            m_skipped++;
        } else if (s == atf::tests::tcr::status_failed) {
            m_failed++;
        } else {
            assert(false);
        }

        m_writer.end_tc(tcr);
    }

    void
    got_stdout_line(const std::string& line)
    {
        m_writer.stdout_tc(line);
    }

    void
    got_stderr_line(const std::string& line)
    {
        m_writer.stderr_tc(line);
    }

public:
    muxer(const atf::fs::path& tp, atf::formats::atf_tps_writer& w,
           atf::io::pistream& is) :
        atf::formats::atf_tcs_reader(is),
        m_tp(tp),
        m_writer(w),
        m_passed(0),
        m_failed(0),
        m_skipped(0)
    {
    }

    ~muxer(void)
    {
        m_writer.end_tp();
    }
};

class atf_run : public atf::application {
    static const char* m_description;

    std::string specific_args(void) const;

    size_t count_tps(std::vector< std::string >) const;

    int run_test(const atf::fs::path&,
                 atf::formats::atf_tps_writer&);
    int run_test_directory(const atf::fs::path&,
                           atf::formats::atf_tps_writer&);
    int run_test_program(const atf::fs::path&,
                         atf::formats::atf_tps_writer&);

    void run_test_program_child(const atf::fs::path&,
                                atf::io::pipe&,
                                atf::io::pipe&,
                                atf::io::pipe&);
    int run_test_program_parent(const atf::fs::path&,
                                atf::formats::atf_tps_writer&,
                                atf::io::pipe&,
                                atf::io::pipe&,
                                atf::io::pipe&,
                                pid_t);

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
atf_run::run_test(const atf::fs::path& tp, atf::formats::atf_tps_writer& w)
{
    atf::fs::file_info fi(tp);

    int errcode;
    if (fi.get_type() == atf::fs::file_info::dir_type)
        errcode = run_test_directory(tp, w);
    else
        errcode = run_test_program(tp, w);
    return errcode;
}

int
atf_run::run_test_directory(const atf::fs::path& tp,
                            atf::formats::atf_tps_writer& w)
{
    atf::atffile af(tp / "Atffile");

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = af.begin();
         iter != af.end(); iter++)
        ok &= (run_test(tp / *iter, w) == EXIT_SUCCESS);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

void
atf_run::run_test_program_child(const atf::fs::path& tp,
                                atf::io::pipe& outpipe,
                                atf::io::pipe& errpipe,
                                atf::io::pipe& respipe)
{
    // Remap stdout and stderr to point to the parent, who will capture
    // everything sent to these.
    outpipe.rend().close();
    outpipe.wend().posix_remap(STDOUT_FILENO);
    errpipe.rend().close();
    errpipe.wend().posix_remap(STDERR_FILENO);

    // Remap the results file descriptor to point to the parent too.
    // We use the 9th one (instead of a bigger one) because shell scripts
    // can only use the [0..9] file descriptors in their redirections.
    respipe.rend().close();
    respipe.wend().posix_remap(9);

    // Prepare the test program's arguments.  We use dynamic memory and
    // do not care to release it.  We are going to die anyway very soon,
    // either due to exec(2) or to exit(3).
    char* args[4];
    {
        // 0: Program name.
        const char* name = tp.leaf_name().c_str();
        args[0] = new char[std::strlen(name) + 1];
        std::strcpy(args[0], name);

        // 1: The file descriptor to which the results will be printed.
        args[1] = new char[4];
        std::strcpy(args[1], "-r9");

        // 2: The directory where the test program lives.
        atf::fs::path bp = tp.branch_path();
        if (!bp.is_absolute())
            bp = bp.to_absolute();
        const char* dir = bp.c_str();
        args[2] = new char[std::strlen(dir) + 3];
        std::strcpy(args[2], "-s");
        std::strcat(args[2], dir);

        // 3: Terminator.
        args[3] = NULL;
    }

    // Do the real exec and report any errors to the parent through the
    // only mechanism we can use: stderr.
    // TODO Try to make this fail.
    ::execv(tp.c_str(), args);
    std::cerr << "Failed to execute `" << tp.str() << "': "
              << std::strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
}

int
atf_run::run_test_program_parent(const atf::fs::path& tp,
                                 atf::formats::atf_tps_writer& w,
                                 atf::io::pipe& outpipe,
                                 atf::io::pipe& errpipe,
                                 atf::io::pipe& respipe,
                                 pid_t pid)
{
    // Get the file descriptor and input stream of stdout.
    outpipe.wend().close();
    atf::io::pistream outin(outpipe.rend());

    // Get the file descriptor and input stream of stderr.
    errpipe.wend().close();
    atf::io::pistream errin(errpipe.rend());

    // Get the file descriptor and input stream of the results channel.
    respipe.wend().close();
    atf::io::pistream resin(respipe.rend());

    // Process the test case's output and multiplex it into our output
    // stream as we read it.
    muxer(tp, w, resin).read(outin, errin);

    outin.close();
    errin.close();
    resin.close();

    int status;
    if (::waitpid(pid, &status, 0) == -1)
        throw atf::system_error("atf_run::run_test_program(" + tp.str() +
                                ")", "waitpid(2) failed", errno);

    int code;
    if (WIFEXITED(status)) {
        code = WEXITSTATUS(status);
        //if (failed > 0 && code == EXIT_SUCCESS) {
            // XXX Bogus test.
        //}
    } else
        code = EXIT_FAILURE;
    return code;
}

int
atf_run::run_test_program(const atf::fs::path& tp,
                          atf::formats::atf_tps_writer& w)
{
    int errcode;

    atf::io::pipe outpipe, errpipe, respipe;
    pid_t pid = ::fork();
    if (pid == -1) {
        throw atf::system_error("run_test_program",
                                "fork(2) failed", errno);
    } else if (pid == 0) {
        run_test_program_child(tp, outpipe, errpipe, respipe);
        // NOTREACHED
        assert(false);
        errcode = EXIT_FAILURE;
    } else {
        errcode = run_test_program_parent(tp, w, outpipe, errpipe, respipe,
                                          pid);
    }

    return errcode;
}

size_t
atf_run::count_tps(std::vector< std::string > tps)
    const
{
    size_t ntps = 0;

    for (std::vector< std::string >::const_iterator iter = tps.begin();
         iter != tps.end(); iter++) {
        atf::fs::path tp(*iter);
        atf::fs::file_info fi(tp);

        if (fi.get_type() == atf::fs::file_info::dir_type) {
            std::vector< std::string > aux = atf::atffile(tp / "Atffile");
            for (std::vector< std::string >::iterator i2 = aux.begin();
                 i2 != aux.end(); i2++)
                *i2 = (tp / *i2).str();
            ntps += count_tps(aux);
        } else
            ntps++;
    }

    return ntps;
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

    atf::formats::atf_tps_writer w(std::cout, count_tps(tps));

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = tps.begin();
         iter != tps.end(); iter++)
        ok &= (run_test(atf::fs::path(*iter), w) == EXIT_SUCCESS);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int
main(int argc, char* const* argv)
{
    return atf_run().run(argc, argv);
}
