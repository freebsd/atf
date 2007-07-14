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
#include <unistd.h>
}

#include <cstdlib>
#include <cstring>

#include <atf.hpp>

#include "atfprivate/file_handle.hpp"

ATF_TEST_CASE(tc_construct);
ATF_TEST_CASE_HEAD(tc_construct)
{
    set("descr", "Tests file_handle's constructors");
}
ATF_TEST_CASE_BODY(tc_construct)
{
    atf::file_handle fh1;
    ATF_CHECK(!fh1.is_valid());

    atf::file_handle fh2(STDERR_FILENO);
    ATF_CHECK(fh2.is_valid());
    fh2.disown();
}

ATF_TEST_CASE(tc_copy);
ATF_TEST_CASE_HEAD(tc_copy)
{
    set("descr", "Tests file_handle's copy constructor");
}
ATF_TEST_CASE_BODY(tc_copy)
{
    atf::file_handle fh1;
    atf::file_handle fh2(STDERR_FILENO);

    atf::file_handle fh3(fh2);
    ATF_CHECK(!fh2.is_valid());
    ATF_CHECK(fh3.is_valid());

    fh1 = fh3;
    ATF_CHECK(!fh3.is_valid());
    ATF_CHECK(fh1.is_valid());

    fh1.disown();
}

ATF_TEST_CASE(tc_get);
ATF_TEST_CASE_HEAD(tc_get)
{
    set("descr", "Tests the file_handle::get method");
}
ATF_TEST_CASE_BODY(tc_get)
{
    atf::file_handle fh1(STDERR_FILENO);
    ATF_CHECK_EQUAL(fh1.get(), STDERR_FILENO);
}

ATF_TEST_CASE(tc_posix_dup);
ATF_TEST_CASE_HEAD(tc_posix_dup)
{
    set("descr", "Tests the file_handle::posix_dup method");
}
ATF_TEST_CASE_BODY(tc_posix_dup)
{
    int pfd[2];

    ATF_CHECK(::pipe(pfd) != -1);
    atf::file_handle rend(pfd[0]);
    atf::file_handle wend(pfd[1]);

    ATF_CHECK(rend.get() != 10);
    ATF_CHECK(wend.get() != 10);
    atf::file_handle fh1 = atf::file_handle::posix_dup(wend.get(), 10);
    ATF_CHECK_EQUAL(fh1.get(), 10);

    ATF_CHECK(::write(wend.get(), "test-posix-dup", 14) != -1);
    char buf1[15];
    ATF_CHECK_EQUAL(::read(rend.get(), buf1, sizeof(buf1)), 14);
    buf1[14] = '\0';
    ATF_CHECK(std::strcmp(buf1, "test-posix-dup") == 0);

    ATF_CHECK(::write(fh1.get(), "test-posix-dup", 14) != -1);
    char buf2[15];
    ATF_CHECK_EQUAL(::read(rend.get(), buf2, sizeof(buf2)), 14);
    buf2[14] = '\0';
    ATF_CHECK(std::strcmp(buf2, "test-posix-dup") == 0);
}

ATF_TEST_CASE(tc_posix_remap);
ATF_TEST_CASE_HEAD(tc_posix_remap)
{
    set("descr", "Tests the file_handle::posix_remap method");
}
ATF_TEST_CASE_BODY(tc_posix_remap)
{
    int pfd[2];

    ATF_CHECK(::pipe(pfd) != -1);
    atf::file_handle rend(pfd[0]);
    atf::file_handle wend(pfd[1]);

    ATF_CHECK(rend.get() != 10);
    ATF_CHECK(wend.get() != 10);
    wend.posix_remap(10);
    ATF_CHECK_EQUAL(wend.get(), 10);
    ATF_CHECK(::write(wend.get(), "test-posix-remap", 16) != -1);

    char buf[17];
    ATF_CHECK_EQUAL(::read(rend.get(), buf, sizeof(buf)), 16);
    buf[16] = '\0';
    ATF_CHECK(std::strcmp(buf, "test-posix-remap") == 0);
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&tc_construct);
    tcs.push_back(&tc_copy);
    tcs.push_back(&tc_get);
    tcs.push_back(&tc_posix_dup);
    tcs.push_back(&tc_posix_remap);
}
