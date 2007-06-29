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
#include <vector>

#include <libatf.hpp>
#include <libatfmain.hpp>

namespace as = atf::shorthands;

class test_pass : public as::tc
{
    void
    head(void)
    {
        set("ident", "tc_pass");
        set("descr", "An empty test case that always passes");
    }

    as::tcr
    body(void)
        const
    {
        return as::tcr::passed();
    }

public:
    test_pass(void)
    {
    }
};

class test_fail : public as::tc
{
    void
    head(void)
    {
        set("ident", "tc_fail");
        set("descr", "An empty test case that always fails");
    }

    as::tcr
    body(void)
        const
    {
        return as::tcr::failed();
    }

public:
    test_fail(void)
    {
    }
};

class test_fail_reason : public as::tc
{
    void
    head(void)
    {
        set("ident", "tc_fail_reason");
        set("descr", "An empty test case that always fails with a reason");
    }

    as::tcr
    body(void)
        const
    {
        return as::tcr::failed("On purpose");
    }

public:
    test_fail_reason(void)
    {
    }
};

class test_skip : public as::tc
{
    void
    head(void)
    {
        set("ident", "tc_skip");
        set("descr", "An empty test case that is always skipped");
    }

    as::tcr
    body(void)
        const
    {
        return as::tcr::skipped("By design");
    }

public:
    test_skip(void)
    {
    }
};

ATF_INIT_TEST_CASES(tcs)
{
    static test_pass test_pass;
    tcs.push_back(&test_pass);

    static test_fail test_fail;
    tcs.push_back(&test_fail);

    static test_fail_reason test_fail_reason;
    tcs.push_back(&test_fail_reason);

    static test_skip test_skip;
    tcs.push_back(&test_skip);
}
