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
#include <sys/stat.h>
#include <fcntl.h>
}

#include <fstream>
#include <stdexcept>

#include "atf/env.hpp"
#include "atf/fs.hpp"
#include "atf/macros.hpp"

static
void
safe_mkdir(const char* path)
{
    if (::mkdir(path, 0755) == -1)
        ATF_FAIL(std::string("mkdir(2) of ") + path + " failed");
}

static
void
touch(const char* path)
{
    std::ofstream os(path);
    if (!os)
        ATF_FAIL(std::string("Could not create file ") + path);
    os.close();
}

ATF_TEST_CASE(env_undef);
ATF_TEST_CASE_HEAD(env_undef)
{
    set("descr", "Helper test case for the t_env test program");
}
ATF_TEST_CASE_BODY(env_undef)
{
    ATF_CHECK(!atf::env::has("LC_COLLATE"));
    ATF_CHECK(!atf::env::has("TZ"));
}

ATF_TEST_CASE(fork_mangle_fds);
ATF_TEST_CASE_HEAD(fork_mangle_fds)
{
    set("descr", "Helper test case for the t_fork test program");

    if (atf::env::has("ISOLATED"))
        set("isolated", atf::env::get("ISOLATED"));
    else
        set("isolated", "yes");
}
ATF_TEST_CASE_BODY(fork_mangle_fds)
{
    if (!atf::env::has("RESFD"))
        ATF_FAIL("RESFD not defined");
    int resfd = std::atoi(atf::env::get("RESFD").c_str());

    if (::close(STDIN_FILENO) == -1)
        ATF_FAIL("Failed to close stdin");
    if (::close(STDOUT_FILENO) == -1)
        ATF_FAIL("Failed to close stdout");
    if (::close(STDERR_FILENO) == -1)
        ATF_FAIL("Failed to close stderr");
    if (::close(resfd) == -1)
        ATF_FAIL("Failed to close results descriptor");

#if defined(F_CLOSEM)
    if (::fcntl(0, F_CLOSEM) == -1)
        ATF_FAIL("Failed to close everything");
#endif
}

ATF_TEST_CASE(isolated_path);
ATF_TEST_CASE_HEAD(isolated_path)
{
    set("descr", "Helper test case for the t_isolated test program");

    if (atf::env::has("ISOLATED"))
        set("isolated", atf::env::get("ISOLATED"));
    else
        set("isolated", "yes");
}
ATF_TEST_CASE_BODY(isolated_path)
{
    if (!atf::env::has("PATHFILE"))
        ATF_FAIL("PATHFILE not defined");
    const std::string& p = atf::env::get("PATHFILE");

    std::ofstream os(p.c_str());
    if (!os)
        ATF_FAIL("Could not open " + p + " for writing");

    os << atf::fs::get_current_dir().str() << std::endl;

    os.close();
}

ATF_TEST_CASE(isolated_cleanup);
ATF_TEST_CASE_HEAD(isolated_cleanup)
{
    set("descr", "Helper test case for the t_isolated test program");
    set("isolated", "yes");
}
ATF_TEST_CASE_BODY(isolated_cleanup)
{
    if (!atf::env::has("PATHFILE"))
        ATF_FAIL("PATHFILE not defined");
    const std::string& p = atf::env::get("PATHFILE");

    std::ofstream os(p.c_str());
    if (!os)
        ATF_FAIL("Could not open " + p + " for writing");

    os << atf::fs::get_current_dir().str() << std::endl;

    os.close();

    safe_mkdir("1");
    safe_mkdir("1/1");
    safe_mkdir("1/2");
    safe_mkdir("1/3");
    safe_mkdir("1/3/1");
    safe_mkdir("1/3/2");
    safe_mkdir("2");
    touch("2/1");
    touch("2/2");
    safe_mkdir("2/3");
    touch("2/3/1");
}

ATF_TEST_CASE(srcdir_exists);
ATF_TEST_CASE_HEAD(srcdir_exists)
{
    set("descr", "Helper test case for the t_srcdir test program");
}
ATF_TEST_CASE_BODY(srcdir_exists)
{
    if (!atf::fs::exists(atf::fs::path(get_srcdir()) / "datafile"))
        ATF_FAIL("Cannot find datafile");
}

ATF_TEST_CASE(require_progs_body);
ATF_TEST_CASE_HEAD(require_progs_body)
{
    set("descr", "Helper test case for the t_require_progs test program");
}
ATF_TEST_CASE_BODY(require_progs_body)
{
    if (atf::env::has("PROGS"))
        require_prog(atf::env::get("PROGS"));
}

ATF_TEST_CASE(require_progs_head);
ATF_TEST_CASE_HEAD(require_progs_head)
{
    set("descr", "Helper test case for the t_require_head test program");
    if (atf::env::has("PROGS"))
        set("require.progs", atf::env::get("PROGS"));
}
ATF_TEST_CASE_BODY(require_progs_head)
{
}

ATF_TEST_CASE(require_user_root);
ATF_TEST_CASE_HEAD(require_user_root)
{
    set("descr", "Helper test case for the t_require_user test program");
    set("isolated", "no");
    set("require.user", "root");
}
ATF_TEST_CASE_BODY(require_user_root)
{
}

ATF_TEST_CASE(require_user_root2);
ATF_TEST_CASE_HEAD(require_user_root2)
{
    set("descr", "Helper test case for the t_require_user test program");
    set("isolated", "no");
    set("require.user", "root");
}
ATF_TEST_CASE_BODY(require_user_root2)
{
}

ATF_TEST_CASE(require_user_unprivileged);
ATF_TEST_CASE_HEAD(require_user_unprivileged)
{
    set("descr", "Helper test case for the t_require_user test program");
    set("isolated", "no");
    set("require.user", "unprivileged");
}
ATF_TEST_CASE_BODY(require_user_unprivileged)
{
}

ATF_TEST_CASE(require_user_unprivileged2);
ATF_TEST_CASE_HEAD(require_user_unprivileged2)
{
    set("descr", "Helper test case for the t_require_user test program");
    set("isolated", "no");
    set("require.user", "unprivileged");
}
ATF_TEST_CASE_BODY(require_user_unprivileged2)
{
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&env_undef);
    tcs.push_back(&fork_mangle_fds);
    tcs.push_back(&isolated_path);
    tcs.push_back(&isolated_cleanup);
    tcs.push_back(&srcdir_exists);
    tcs.push_back(&require_progs_body);
    tcs.push_back(&require_progs_head);
    tcs.push_back(&require_user_root);
    tcs.push_back(&require_user_root2);
    tcs.push_back(&require_user_unprivileged);
    tcs.push_back(&require_user_unprivileged2);
}
