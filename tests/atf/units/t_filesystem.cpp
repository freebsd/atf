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

#include <atf.hpp>

#include "atfprivate/filesystem.hpp"

ATF_TEST_CASE(tc_get_branch_path);

ATF_TEST_CASE_HEAD(tc_get_branch_path)
{
    set("descr", "Tests the get_branch_path function");
}

ATF_TEST_CASE_BODY(tc_get_branch_path)
{
    ATF_CHECK_EQUAL(atf::get_branch_path(""), ".");
    ATF_CHECK_EQUAL(atf::get_branch_path("."), ".");
    ATF_CHECK_EQUAL(atf::get_branch_path("foo"), ".");
    ATF_CHECK_EQUAL(atf::get_branch_path("foo/bar"), "foo");
    ATF_CHECK_EQUAL(atf::get_branch_path("/foo"), "/");
    ATF_CHECK_EQUAL(atf::get_branch_path("/foo/bar"), "/foo");
}

ATF_TEST_CASE(tc_get_leaf_name);

ATF_TEST_CASE_HEAD(tc_get_leaf_name)
{
    set("descr", "Tests the get_leaf_name function");
}

ATF_TEST_CASE_BODY(tc_get_leaf_name)
{
    ATF_CHECK_EQUAL(atf::get_leaf_name(""), ".");
    ATF_CHECK_EQUAL(atf::get_leaf_name("."), ".");
    ATF_CHECK_EQUAL(atf::get_leaf_name("foo"), "foo");
    ATF_CHECK_EQUAL(atf::get_leaf_name("foo/bar"), "bar");
    ATF_CHECK_EQUAL(atf::get_leaf_name("/foo"), "foo");
    ATF_CHECK_EQUAL(atf::get_leaf_name("/foo/bar"), "bar");
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&tc_get_branch_path);
    tcs.push_back(&tc_get_leaf_name);
}
