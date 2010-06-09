//
// Automated Testing Framework (atf)
//
// Copyright (c) 2010 The NetBSD Foundation, Inc.
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

#include <fstream>

#include "atf-c++/macros.hpp"
#include "atf-c++/parser.hpp"

#include "test-program.hpp"

namespace impl = atf::atf_run;

using atf::tests::vars_map;

// -------------------------------------------------------------------------
// Auxiliary functions.
// -------------------------------------------------------------------------

static
atf::fs::path
get_helper(const atf::tests::tc& tc, const char* name)
{
    return atf::fs::path(tc.get_config_var("srcdir")) / name;
}

static
void
check_property(const vars_map& props, const char* name, const char* value)
{
    const vars_map::const_iterator iter = props.find(name);
    ATF_CHECK(iter != props.end());
    ATF_CHECK_EQUAL(value, (*iter).second);
}

static
void
write_test_case_result(const char *results_path, const std::string& contents)
{
    std::ofstream results_file(results_path);
    ATF_CHECK(results_file);

    results_file << contents;
}

// -------------------------------------------------------------------------
// Tests.
// -------------------------------------------------------------------------

ATF_TEST_CASE(get_metadata_bad);
ATF_TEST_CASE_HEAD(get_metadata_bad) {}
ATF_TEST_CASE_BODY(get_metadata_bad) {
    const atf::fs::path executable = get_helper(*this, "h_bad_metadata");
    ATF_CHECK_THROW(impl::get_metadata(executable, vars_map()),
                    atf::parser::parse_errors);
}

ATF_TEST_CASE(get_metadata_zero_tcs);
ATF_TEST_CASE_HEAD(get_metadata_zero_tcs) {}
ATF_TEST_CASE_BODY(get_metadata_zero_tcs) {
    const atf::fs::path executable = get_helper(*this, "h_zero_tcs");
    ATF_CHECK_THROW(impl::get_metadata(executable, vars_map()),
                    atf::parser::parse_errors);
}

ATF_TEST_CASE(get_metadata_several_tcs);
ATF_TEST_CASE_HEAD(get_metadata_several_tcs) {}
ATF_TEST_CASE_BODY(get_metadata_several_tcs) {
    const atf::fs::path executable = get_helper(*this, "h_several_tcs");
    const impl::metadata md = impl::get_metadata(executable, vars_map());
    ATF_CHECK_EQUAL(3, md.test_cases.size());

    {
        const impl::test_cases_map::const_iterator iter =
            md.test_cases.find("first");
        ATF_CHECK(iter != md.test_cases.end());

        ATF_CHECK_EQUAL(5, (*iter).second.size());
        check_property((*iter).second, "descr", "Description 1");
        check_property((*iter).second, "has.cleanup", "false");
        check_property((*iter).second, "ident", "first");
        check_property((*iter).second, "timeout", "300");
        check_property((*iter).second, "use.fs", "false");
    }

    {
        const impl::test_cases_map::const_iterator iter =
            md.test_cases.find("second");
        ATF_CHECK(iter != md.test_cases.end());

        ATF_CHECK_EQUAL(6, (*iter).second.size());
        check_property((*iter).second, "descr", "Description 2");
        check_property((*iter).second, "has.cleanup", "true");
        check_property((*iter).second, "ident", "second");
        check_property((*iter).second, "timeout", "500");
        check_property((*iter).second, "use.fs", "true");
        check_property((*iter).second, "X-property", "Custom property");
    }

    {
        const impl::test_cases_map::const_iterator iter =
            md.test_cases.find("third");
        ATF_CHECK(iter != md.test_cases.end());

        ATF_CHECK_EQUAL(4, (*iter).second.size());
        check_property((*iter).second, "has.cleanup", "false");
        check_property((*iter).second, "ident", "third");
        check_property((*iter).second, "timeout", "300");
        check_property((*iter).second, "use.fs", "false");
    }
}

ATF_TEST_CASE(read_test_case_result_no_file);
ATF_TEST_CASE_HEAD(read_test_case_result_no_file) {}
ATF_TEST_CASE_BODY(read_test_case_result_no_file) {
    ATF_CHECK_THROW(impl::read_test_case_result(atf::fs::path("resfile")),
                    std::runtime_error);
}

ATF_TEST_CASE(read_test_case_result_empty_file);
ATF_TEST_CASE_HEAD(read_test_case_result_empty_file) {
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(read_test_case_result_empty_file) {
    write_test_case_result("resfile", "");
    ATF_CHECK_THROW(impl::read_test_case_result(atf::fs::path("resfile")),
                    std::runtime_error);
}

ATF_TEST_CASE(read_test_case_result_invalid);
ATF_TEST_CASE_HEAD(read_test_case_result_invalid) {
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(read_test_case_result_invalid) {
    write_test_case_result("resfile", "passed: hello\n");
    ATF_CHECK_THROW(impl::read_test_case_result(atf::fs::path("resfile")),
                    std::runtime_error);
}

ATF_TEST_CASE(read_test_case_result_passed);
ATF_TEST_CASE_HEAD(read_test_case_result_passed) {
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(read_test_case_result_passed) {
    write_test_case_result("resfile", "passed\n");
    const atf::tests::tcr tcr = impl::read_test_case_result(atf::fs::path(
        "resfile"));
    ATF_CHECK_EQUAL(atf::tests::tcr::passed_state, tcr.get_state());
}

ATF_TEST_CASE(read_test_case_result_failed);
ATF_TEST_CASE_HEAD(read_test_case_result_failed) {
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(read_test_case_result_failed) {
    write_test_case_result("resfile", "failed: foo bar\n");
    const atf::tests::tcr tcr = impl::read_test_case_result(atf::fs::path(
        "resfile"));
    ATF_CHECK_EQUAL(atf::tests::tcr::failed_state, tcr.get_state());
    ATF_CHECK_EQUAL("foo bar", tcr.get_reason());
}

ATF_TEST_CASE(read_test_case_result_skipped);
ATF_TEST_CASE_HEAD(read_test_case_result_skipped) {
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(read_test_case_result_skipped) {
    write_test_case_result("resfile", "skipped: baz bar\n");
    const atf::tests::tcr tcr = impl::read_test_case_result(atf::fs::path(
        "resfile"));
    ATF_CHECK_EQUAL(atf::tests::tcr::skipped_state, tcr.get_state());
    ATF_CHECK_EQUAL("baz bar", tcr.get_reason());
}

ATF_TEST_CASE(read_test_case_result_multiline);
ATF_TEST_CASE_HEAD(read_test_case_result_multiline) {
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(read_test_case_result_multiline) {
    write_test_case_result("resfile", "skipped: foo\nbar\n");
    const atf::tests::tcr tcr = impl::read_test_case_result(atf::fs::path(
        "resfile"));
    ATF_CHECK_EQUAL(atf::tests::tcr::skipped_state, tcr.get_state());
    ATF_CHECK_EQUAL("foo<<NEWLINE UNEXPECTED>>bar", tcr.get_reason());
}

// -------------------------------------------------------------------------
// Main.
// -------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    ATF_ADD_TEST_CASE(tcs, get_metadata_bad);
    ATF_ADD_TEST_CASE(tcs, get_metadata_zero_tcs);
    ATF_ADD_TEST_CASE(tcs, get_metadata_several_tcs);

    ATF_ADD_TEST_CASE(tcs, read_test_case_result_no_file);
    ATF_ADD_TEST_CASE(tcs, read_test_case_result_empty_file);
    ATF_ADD_TEST_CASE(tcs, read_test_case_result_multiline);
    ATF_ADD_TEST_CASE(tcs, read_test_case_result_invalid);
    ATF_ADD_TEST_CASE(tcs, read_test_case_result_passed);
    ATF_ADD_TEST_CASE(tcs, read_test_case_result_failed);
    ATF_ADD_TEST_CASE(tcs, read_test_case_result_skipped);

    // TODO: Add tests for run_test_case once all the missing functionality
    // is implemented.
}
