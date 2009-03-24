//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007, 2008, 2009 The NetBSD Foundation, Inc.
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

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include <atf-c++.hpp>

#include "atf-c++/check.hpp"
#include "atf-c++/config.hpp"
#include "atf-c++/fs.hpp"
#include "atf-c++/text.hpp"
#include "atf-c++/utils.hpp"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

extern "C" {
atf_error_t atf_check_result_init(atf_check_result_t *, const char* const*);
}

namespace atf {
namespace check {

check_result
test_constructor(const char* const* argv)
{
    atf_check_result_t r;
    atf_check_result_init(&r, argv);

    return atf::check::check_result(&r);
}

} // namespace check
} // namespace atf

static
atf::check::check_result
do_exec(const atf::tests::tc* tc, const char* helper_name)
{
    std::vector< std::string > argv;
    argv.push_back(tc->get_config_var("srcdir") + "/../atf-c/h_check");
    argv.push_back(helper_name);
    std::cout << "Executing " << argv[0] << " " << argv[1] << "\n";

    atf::check::argv_array argva(argv);
    return atf::check::exec(argva);
}

static
atf::check::check_result
do_exec(const atf::tests::tc* tc, const char* helper_name, const char *carg2)
{
    std::vector< std::string > argv;
    argv.push_back(tc->get_config_var("srcdir") + "/../atf-c/h_check");
    argv.push_back(helper_name);
    argv.push_back(carg2);
    std::cout << "Executing " << argv[0] << " " << argv[1] << " "
              << argv[2] << "\n";

    atf::check::argv_array argva(argv);
    return atf::check::exec(argva);
}

static
std::size_t
array_size(const char* const* array)
{
    std::size_t size = 0;

    for (const char* const* ptr = array; *ptr != NULL; ptr++)
        size++;

    return size;
}

// ------------------------------------------------------------------------
// Tests for the "argv_array" type.
// ------------------------------------------------------------------------

ATF_TEST_CASE(argv_array_ext);
ATF_TEST_CASE_HEAD(argv_array_ext)
{
    set_md_var("descr", "Tests that argv_array can hold an external, "
               "constant copy of an array of strings");
}
ATF_TEST_CASE_BODY(argv_array_ext)
{
    {
        const char* const argv[] = { NULL };
        atf::check::argv_array argva(argv);

        const char* const* eargv = argva.to_exec_argv();
        ATF_CHECK_EQUAL(array_size(argv), array_size(eargv));
        ATF_CHECK_EQUAL(argv, eargv);
        ATF_CHECK_EQUAL(argv[0], eargv[0]);
    }

    {
        const char* const argv[] = { "arg0", NULL };
        atf::check::argv_array argva(argv);

        const char* const* eargv = argva.to_exec_argv();
        ATF_CHECK_EQUAL(array_size(argv), array_size(eargv));
        ATF_CHECK_EQUAL(argv, eargv);
        ATF_CHECK_EQUAL(argv[0], eargv[0]);
        ATF_CHECK_EQUAL(argv[1], eargv[1]);
    }

    {
        const char* const argv[] = { "arg0", "arg1", "arg2", NULL };
        atf::check::argv_array argva(argv);

        const char* const* eargv = argva.to_exec_argv();
        ATF_CHECK_EQUAL(array_size(argv), array_size(eargv));
        ATF_CHECK_EQUAL(argv, eargv);
        ATF_CHECK_EQUAL(argv[0], eargv[0]);
        ATF_CHECK_EQUAL(argv[1], eargv[1]);
        ATF_CHECK_EQUAL(argv[2], eargv[2]);
        ATF_CHECK_EQUAL(argv[3], eargv[3]);
    }
}

ATF_TEST_CASE(argv_array_int);
ATF_TEST_CASE_HEAD(argv_array_int)
{
    set_md_var("descr", "Tests that argv_array can hold a copy of a "
               "collection representing an array of arguments");
}
ATF_TEST_CASE_BODY(argv_array_int)
{
    {
        std::vector< std::string > argv;
        atf::check::argv_array argva(argv);

        const char* const* eargv = argva.to_exec_argv();
        ATF_CHECK_EQUAL(argv.size(), array_size(eargv));
        ATF_CHECK_EQUAL(eargv[0], NULL);
    }

    {
        std::string arg0 = "arg0";
        std::vector< std::string > argv;
        argv.push_back(arg0);
        atf::check::argv_array argva(argv);

        const char* const* eargv = argva.to_exec_argv();
        ATF_CHECK_EQUAL(argv.size(), array_size(eargv));
        ATF_CHECK(eargv[0] != arg0.c_str());
        ATF_CHECK_EQUAL(std::string(eargv[0]), arg0);
        ATF_CHECK_EQUAL(eargv[1], NULL);
    }

    {
        std::string arg0 = "arg0";
        std::string arg1 = "arg1";
        std::string arg2 = "arg2";
        std::vector< std::string > argv;
        argv.push_back(arg0);
        argv.push_back(arg1);
        argv.push_back(arg2);
        atf::check::argv_array argva(argv);

        const char* const* eargv = argva.to_exec_argv();
        ATF_CHECK_EQUAL(argv.size(), array_size(eargv));
        ATF_CHECK(eargv[0] != arg0.c_str());
        ATF_CHECK_EQUAL(std::string(eargv[0]), arg0);
        ATF_CHECK(eargv[1] != arg1.c_str());
        ATF_CHECK_EQUAL(std::string(eargv[1]), arg1);
        ATF_CHECK(eargv[2] != arg2.c_str());
        ATF_CHECK_EQUAL(std::string(eargv[2]), arg2);
        ATF_CHECK_EQUAL(eargv[3], NULL);
    }
}

// ------------------------------------------------------------------------
// Tests for the "check_result" type.
// ------------------------------------------------------------------------

ATF_TEST_CASE(result_argv);
ATF_TEST_CASE_HEAD(result_argv)
{
    set_md_var("descr", "Tests that check_result contains a valid copy of "
               "argv");
}
ATF_TEST_CASE_BODY(result_argv)
{
    const char *const expargv[] = {
        "progname",
        "arg1",
        "arg2",
        NULL
    };

    const atf::check::check_result result =
        atf::check::test_constructor(expargv);

    ATF_CHECK_EQUAL(result.argv().size(), 3);
    ATF_CHECK_EQUAL(result.argv()[0], "progname");
    ATF_CHECK_EQUAL(result.argv()[1], "arg1");
    ATF_CHECK_EQUAL(result.argv()[2], "arg2");
}

ATF_TEST_CASE(result_templates);
ATF_TEST_CASE_HEAD(result_templates)
{
    set_md_var("descr", "Tests that check_result is initialized with "
               "correct temporary file templates");
}
ATF_TEST_CASE_BODY(result_templates)
{
    const char *const argv[] = { "fake", NULL };

    const atf::check::check_result result1 =
        atf::check::test_constructor(argv);
    const atf::check::check_result result2 =
        atf::check::test_constructor(argv);

    const atf::fs::path& out1 = result1.stdout_path();
    const atf::fs::path& err1 = result1.stderr_path();
    const atf::fs::path& out2 = result2.stdout_path();
    const atf::fs::path& err2 = result2.stderr_path();

    ATF_CHECK(out1.str().find("stdout.XXXXXX") != std::string::npos);
    ATF_CHECK(err1.str().find("stderr.XXXXXX") != std::string::npos);
    ATF_CHECK(out2.str().find("stdout.XXXXXX") != std::string::npos);
    ATF_CHECK(err2.str().find("stderr.XXXXXX") != std::string::npos);

    ATF_CHECK(out1 == out2);
    ATF_CHECK(err1 == err2);
}

// ------------------------------------------------------------------------
// Test cases for the free functions.
// ------------------------------------------------------------------------

ATF_TEST_CASE(exec_argv);
ATF_TEST_CASE_HEAD(exec_argv)
{
    set_md_var("descr", "Tests that exec preserves the provided argv");
}
ATF_TEST_CASE_BODY(exec_argv)
{
    const atf::check::check_result r = do_exec(this, "exit-success");
    ATF_CHECK_EQUAL(r.argv().size(), 2);
    ATF_CHECK_EQUAL(r.argv()[0],
                    get_config_var("srcdir") + "/../atf-c/h_check");
    ATF_CHECK_EQUAL(r.argv()[1], "exit-success");
}

ATF_TEST_CASE(exec_cleanup);
ATF_TEST_CASE_HEAD(exec_cleanup)
{
    set_md_var("descr", "Tests that exec properly cleans up the temporary "
               "files it creates");
}
ATF_TEST_CASE_BODY(exec_cleanup)
{
    std::auto_ptr< atf::fs::path > out;
    std::auto_ptr< atf::fs::path > err;

    {
        const atf::check::check_result r = do_exec(this, "exit-success");
        out.reset(new atf::fs::path(r.stdout_path()));
        err.reset(new atf::fs::path(r.stderr_path()));
        ATF_CHECK(atf::fs::exists(*out.get()));
        ATF_CHECK(atf::fs::exists(*err.get()));
    }
    ATF_CHECK(!atf::fs::exists(*out.get()));
    ATF_CHECK(!atf::fs::exists(*err.get()));
}

ATF_TEST_CASE(exec_exitstatus);
ATF_TEST_CASE_HEAD(exec_exitstatus)
{
    set_md_var("descr", "Tests that exec properly captures the exit "
               "status of the executed command");
}
ATF_TEST_CASE_BODY(exec_exitstatus)
{
    {
        atf::check::check_result r = do_exec(this, "exit-success");
        ATF_CHECK(r.exited());
        ATF_CHECK_EQUAL(r.exitcode(), EXIT_SUCCESS);
    }

    {
        atf::check::check_result r = do_exec(this, "exit-failure");
        ATF_CHECK(r.exited());
        ATF_CHECK_EQUAL(r.exitcode(), EXIT_FAILURE);
    }

    {
        atf::check::check_result r = do_exec(this, "exit-signal");
        ATF_CHECK(!r.exited());
    }
}

static
void
check_lines(const atf::fs::path& path, const char* outname,
            const char* resname)
{
    std::ifstream f(path.c_str());
    ATF_CHECK(f);

    std::string line;
    std::getline(f, line);
    ATF_CHECK_EQUAL(line, std::string("Line 1 to ") + outname + " for " +
                    resname);
    std::getline(f, line);
    ATF_CHECK_EQUAL(line, std::string("Line 2 to ") + outname + " for " +
                    resname);
}

ATF_TEST_CASE(exec_stdout_stderr);
ATF_TEST_CASE_HEAD(exec_stdout_stderr)
{
    set_md_var("descr", "Tests that exec properly captures the stdout "
               "and stderr streams of the child process");
}
ATF_TEST_CASE_BODY(exec_stdout_stderr)
{
    const atf::check::check_result r1 = do_exec(this, "stdout-stderr",
                                                "result1");
    ATF_CHECK(r1.exited());
    ATF_CHECK_EQUAL(r1.exitcode(), EXIT_SUCCESS);

    const atf::check::check_result r2 = do_exec(this, "stdout-stderr",
                                                "result2");
    ATF_CHECK(r2.exited());
    ATF_CHECK_EQUAL(r2.exitcode(), EXIT_SUCCESS);

    const atf::fs::path& out1 = r1.stdout_path();
    const atf::fs::path& out2 = r2.stdout_path();
    const atf::fs::path& err1 = r1.stderr_path();
    const atf::fs::path& err2 = r2.stderr_path();

    ATF_CHECK(out1.str().find("stdout.XXXXXX") == std::string::npos);
    ATF_CHECK(out2.str().find("stdout.XXXXXX") == std::string::npos);
    ATF_CHECK(err1.str().find("stderr.XXXXXX") == std::string::npos);
    ATF_CHECK(err2.str().find("stderr.XXXXXX") == std::string::npos);

    ATF_CHECK(out1 != out2);
    ATF_CHECK(err1 != err2);

    check_lines(out1, "stdout", "result1");
    check_lines(out2, "stdout", "result2");
    check_lines(err1, "stderr", "result1");
    check_lines(err2, "stderr", "result2");
}

ATF_TEST_CASE(exec_unknown);
ATF_TEST_CASE_HEAD(exec_unknown)
{
    set_md_var("descr", "Tests that running a non-existing binary "
               "is handled correctly");
}
ATF_TEST_CASE_BODY(exec_unknown)
{
    std::vector< std::string > argv;
    argv.push_back(atf::config::get("atf_workdir") + "/non-existent");

    atf::check::argv_array argva(argv);
    const atf::check::check_result r = atf::check::exec(argva);
    ATF_CHECK(r.exited());
    ATF_CHECK_EQUAL(r.exitcode(), 127);
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the test cases for the "argv_array" type.
    ATF_ADD_TEST_CASE(tcs, argv_array_ext);
    ATF_ADD_TEST_CASE(tcs, argv_array_int);

    // Add the test cases for the "check_result" type.
    ATF_ADD_TEST_CASE(tcs, result_argv);
    ATF_ADD_TEST_CASE(tcs, result_templates);

    // Add the test cases for the free functions.
    ATF_ADD_TEST_CASE(tcs, exec_argv);
    ATF_ADD_TEST_CASE(tcs, exec_cleanup);
    ATF_ADD_TEST_CASE(tcs, exec_exitstatus);
    ATF_ADD_TEST_CASE(tcs, exec_stdout_stderr);
    ATF_ADD_TEST_CASE(tcs, exec_unknown);
}
