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

#include <sstream>

#include <atf.hpp>

ATF_TEST_CASE(default_ctor);
ATF_TEST_CASE_HEAD(default_ctor)
{
    set("descr", "Tests that the default constructor creates a failed "
                 "result.");
}
ATF_TEST_CASE_BODY(default_ctor)
{
    using atf::test_case_result;

    test_case_result tcr;
    ATF_CHECK(tcr.get_status() == test_case_result::status_failed);
}

ATF_TEST_CASE(passed_ctor);
ATF_TEST_CASE_HEAD(passed_ctor)
{
    set("descr", "Tests that the passed pseudo-constructor works.");
}
ATF_TEST_CASE_BODY(passed_ctor)
{
    using atf::test_case_result;

    test_case_result tcr = test_case_result::passed();
    ATF_CHECK(tcr.get_status() == test_case_result::status_passed);
}

ATF_TEST_CASE(skipped_ctor);
ATF_TEST_CASE_HEAD(skipped_ctor)
{
    set("descr", "Tests that the skipped pseudo-constructor works.");
}
ATF_TEST_CASE_BODY(skipped_ctor)
{
    using atf::test_case_result;

    {
        test_case_result tcr = test_case_result::skipped("Reason 1");
        ATF_CHECK(tcr.get_status() == test_case_result::status_skipped);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Reason 1");
    }

    {
        test_case_result tcr = test_case_result::skipped("Reason 2");
        ATF_CHECK(tcr.get_status() == test_case_result::status_skipped);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Reason 2");
    }
}

ATF_TEST_CASE(failed_ctor);
ATF_TEST_CASE_HEAD(failed_ctor)
{
    set("descr", "Tests that the failed pseudo-constructor works.");
}
ATF_TEST_CASE_BODY(failed_ctor)
{
    using atf::test_case_result;

    {
        test_case_result tcr = test_case_result::failed("Reason 1");
        ATF_CHECK(tcr.get_status() == test_case_result::status_failed);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Reason 1");
    }

    {
        test_case_result tcr = test_case_result::failed("Reason 2");
        ATF_CHECK(tcr.get_status() == test_case_result::status_failed);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Reason 2");
    }
}

ATF_TEST_CASE(externalize);
ATF_TEST_CASE_HEAD(externalize)
{
    set("descr", "Tests that the externalization works and follows the "
                 "expected format.");
}
ATF_TEST_CASE_BODY(externalize)
{
    using atf::test_case_result;

    {
        std::ostringstream ss;
        test_case_result tcr = test_case_result::passed();

        ss << tcr;
        ATF_CHECK_EQUAL(ss.str(), "passed\n");
    }

    {
        std::ostringstream ss;
        test_case_result tcr = test_case_result::skipped("Some reason");

        ss << tcr;
        ATF_CHECK_EQUAL(ss.str(), "skipped, Some reason\n");
    }

    {
        std::ostringstream ss;
        test_case_result tcr = test_case_result::skipped("Foo, bar, baz");

        ss << tcr;
        ATF_CHECK_EQUAL(ss.str(), "skipped, Foo, bar, baz\n");
    }

    {
        std::ostringstream ss;
        test_case_result tcr = test_case_result::failed("Some reason");

        ss << tcr;
        ATF_CHECK_EQUAL(ss.str(), "failed, Some reason\n");
    }

    {
        std::ostringstream ss;
        test_case_result tcr = test_case_result::failed("Foo, bar, baz");

        ss << tcr;
        ATF_CHECK_EQUAL(ss.str(), "failed, Foo, bar, baz\n");
    }
}

ATF_TEST_CASE(internalize);
ATF_TEST_CASE_HEAD(internalize)
{
    set("descr", "Tests that the internalization works and follows the "
                 "expected format.");
}
ATF_TEST_CASE_BODY(internalize)
{
    using atf::format_error;
    using atf::test_case_result;

    {
        std::istringstream ss("");
        test_case_result tcr;

        ATF_CHECK_THROW(ss >> tcr, format_error);
    }

    {
        std::istringstream ss("foo");
        test_case_result tcr;

        ATF_CHECK_THROW(ss >> tcr, format_error);
    }

    {
        std::istringstream ss("passed");
        test_case_result tcr;

        ss >> tcr;
        ATF_CHECK(tcr.get_status() == test_case_result::status_passed);
    }

    {
        std::istringstream ss("skipped");
        test_case_result tcr;

        ATF_CHECK_THROW(ss >> tcr, format_error);
    }

    {
        std::istringstream ss("skipped, Foo bar");
        test_case_result tcr;

        ss >> tcr;
        ATF_CHECK(tcr.get_status() == test_case_result::status_skipped);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Foo bar");
    }

    {
        std::istringstream ss("skipped, Foo, bar, baz");
        test_case_result tcr;

        ss >> tcr;
        ATF_CHECK(tcr.get_status() == test_case_result::status_skipped);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Foo, bar, baz");
    }

    {
        std::istringstream ss("failed");
        test_case_result tcr;

        ATF_CHECK_THROW(ss >> tcr, format_error);
    }

    {
        std::istringstream ss("failed, Foo bar");
        test_case_result tcr;

        ss >> tcr;
        ATF_CHECK(tcr.get_status() == test_case_result::status_failed);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Foo bar");
    }

    {
        std::istringstream ss("failed, Foo, bar, baz");
        test_case_result tcr;

        ss >> tcr;
        ATF_CHECK(tcr.get_status() == test_case_result::status_failed);
        ATF_CHECK_EQUAL(tcr.get_reason(), "Foo, bar, baz");
    }
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&default_ctor);
    tcs.push_back(&passed_ctor);
    tcs.push_back(&skipped_ctor);
    tcs.push_back(&failed_ctor);

    tcs.push_back(&externalize);
    tcs.push_back(&internalize);
}
