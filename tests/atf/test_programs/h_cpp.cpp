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
}

#include <cstdlib>
#include <fstream>
#include <stdexcept>

#include <atf.hpp>

#include "atfprivate/fs.hpp"

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

ATF_TEST_CASE(isolated_path);
ATF_TEST_CASE_HEAD(isolated_path)
{
    set("descr", "Helper test case for the t_isolated test program");

    const char* i = std::getenv("ISOLATED");
    if (i == NULL)
        set("isolated", "yes");
    else
        set("isolated", i);
}
ATF_TEST_CASE_BODY(isolated_path)
{
    const char* p = std::getenv("PATHFILE");
    if (p == NULL)
        ATF_FAIL("PATHFILE not defined");

    std::ofstream os(p);
    if (!os)
        ATF_FAIL(std::string("Could not open ") + p + " for writing");

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
    const char* p = std::getenv("PATHFILE");
    if (p == NULL)
        ATF_FAIL("PATHFILE not defined");

    std::ofstream os(p);
    if (!os)
        ATF_FAIL(std::string("Could not open ") + p + " for writing");

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
    tcs.push_back(&isolated_path);
    tcs.push_back(&isolated_cleanup);
    tcs.push_back(&srcdir_exists);
    tcs.push_back(&require_user_root);
    tcs.push_back(&require_user_root2);
    tcs.push_back(&require_user_unprivileged);
    tcs.push_back(&require_user_unprivileged2);
}
