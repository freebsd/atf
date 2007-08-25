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

#include <sstream>

#include "atf/macros.hpp"
#include "atf/serial.hpp"

// ------------------------------------------------------------------------
// Tests for the "externalizer" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(ext_empty);
ATF_TEST_CASE_HEAD(ext_empty)
{
    set("descr", "Tests the externalizer class without any data");
}
ATF_TEST_CASE_BODY(ext_empty)
{
    using atf::serial::externalizer;

    {
        std::ostringstream ss;
        externalizer e(ss, "text/X-test", 0);
        e.flush();

        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "\n";

        ATF_CHECK_EQUAL(ss.str(), str);
    }

    {
        std::ostringstream ss;
        externalizer e(ss, "text/X-test", 0);
        e.flush();

        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "\n";

        ATF_CHECK_EQUAL(ss.str(), str);
    }

    {
        std::ostringstream ss;
        externalizer e(ss, "text/X-test2", 123);
        e.flush();

        std::string str;
        str += "Content-Type: text/X-test2; version=\"123\"\n";
        str += "\n";

        ATF_CHECK_EQUAL(ss.str(), str);
    }
}

ATF_TEST_CASE(ext_attrs);
ATF_TEST_CASE_HEAD(ext_attrs)
{
    set("descr", "Tests the externalizer class using some extra "
                 "attributes");
}
ATF_TEST_CASE_BODY(ext_attrs)
{
    using atf::serial::attrs_map;
    using atf::serial::externalizer;
    using atf::serial::header_entry;

    std::ostringstream ss;
    externalizer e(ss, "text/X-test", 0);
    attrs_map attrs;
    attrs["foo"] = "bar";
    attrs["some"] = "thing";
    attrs["a"] = "b";
    e.add_header(header_entry("Test", "value", attrs));
    e.flush();

    std::string str;
    str += "Content-Type: text/X-test; version=\"0\"\n";
    str += "Test: value; a=\"b\"; foo=\"bar\"; some=\"thing\"\n";
    str += "\n";

    ATF_CHECK_EQUAL(ss.str(), str);
}

ATF_TEST_CASE(ext_data);
ATF_TEST_CASE_HEAD(ext_data)
{
    set("descr", "Tests the externalizer class with some data");
}
ATF_TEST_CASE_BODY(ext_data)
{
    using atf::serial::externalizer;

    std::ostringstream ss;
    externalizer e(ss, "text/X-test", 0);
    e.putline("This is a line");
    e.putline(123);

    std::string str;
    str += "Content-Type: text/X-test; version=\"0\"\n";
    str += "\n";
    str += "This is a line\n";
    str += "123\n";

    ATF_CHECK_EQUAL(ss.str(), str);
}

// ------------------------------------------------------------------------
// Tests for the "internalizer" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(int_empty);
ATF_TEST_CASE_HEAD(int_empty)
{
    set("descr", "Tests the internalizer class without any data");
}
ATF_TEST_CASE_BODY(int_empty)
{
    using atf::serial::header_entry;
    using atf::serial::internalizer;

    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "\n";

        std::istringstream ss(str);
        internalizer i(ss, "text/X-test", 0);

        ATF_CHECK(i.has_header("Content-Type"));
        const header_entry& ct = i.get_header("Content-Type");
        ATF_CHECK_EQUAL(ct.value(), "text/X-test");
        ATF_CHECK(ct.has_attr("version"));
        ATF_CHECK_EQUAL(ct.get_attr("version"), "0");
    }

    {
        std::string str;
        str += "Content-Type: text/X-test2; version=\"123\"\n";
        str += "\n";

        std::istringstream ss(str);
        internalizer i(ss, "text/X-test2", 123);

        ATF_CHECK(i.has_header("Content-Type"));
        const header_entry& ct = i.get_header("Content-Type");
        ATF_CHECK_EQUAL(ct.value(), "text/X-test2");
        ATF_CHECK(ct.has_attr("version"));
        ATF_CHECK_EQUAL(ct.get_attr("version"), "123");
    }
}

ATF_TEST_CASE(int_attrs);
ATF_TEST_CASE_HEAD(int_attrs)
{
    set("descr", "Tests the internalizer class using some extra "
                 "attributes");
}
ATF_TEST_CASE_BODY(int_attrs)
{
    using atf::serial::header_entry;
    using atf::serial::internalizer;

    std::string str;
    str += "Content-Type: text/X-test; version=\"0\"\n";
    str += "Test: value; a=b; foo=\"bar\"; some=\"thing\"\n";
    str += "Quoting: value; a=\"b\\\"c\"; d=\"e\\\\f\"\n";
    str += "\n";

    std::istringstream ss(str);
    internalizer i(ss, "text/X-test", 0);

    ATF_CHECK(i.has_header("Content-Type"));
    const header_entry& ct = i.get_header("Content-Type");
    ATF_CHECK_EQUAL(ct.value(), "text/X-test");
    ATF_CHECK(ct.has_attr("version"));
    ATF_CHECK_EQUAL(ct.get_attr("version"), "0");

    {
        ATF_CHECK(i.has_header("Test"));
        const header_entry& he = i.get_header("Test");
        ATF_CHECK_EQUAL(he.value(), "value");
        ATF_CHECK(he.has_attr("a"));
        ATF_CHECK_EQUAL(he.get_attr("a"), "b");
        ATF_CHECK(he.has_attr("foo"));
        ATF_CHECK_EQUAL(he.get_attr("foo"), "bar");
        ATF_CHECK(he.has_attr("some"));
        ATF_CHECK_EQUAL(he.get_attr("some"), "thing");
    }

    {
        ATF_CHECK(i.has_header("Quoting"));
        const header_entry& he = i.get_header("Quoting");
        ATF_CHECK_EQUAL(he.value(), "value");
        ATF_CHECK(he.has_attr("a"));
        ATF_CHECK_EQUAL(he.get_attr("a"), "b\"c");
        ATF_CHECK(he.has_attr("d"));
        ATF_CHECK_EQUAL(he.get_attr("d"), "e\\f");
    }
}

ATF_TEST_CASE(int_data);
ATF_TEST_CASE_HEAD(int_data)
{
    set("descr", "Tests the internalizer class with some data");
}
ATF_TEST_CASE_BODY(int_data)
{
    using atf::serial::header_entry;
    using atf::serial::internalizer;

    std::string str;
    str += "Content-Type: text/X-test; version=\"0\"\n";
    str += "\n";
    str += "Word\n";
    str += "123\n";

    std::istringstream ss(str);
    internalizer i(ss, "text/X-test", 0);

    ATF_CHECK(i.has_header("Content-Type"));
    const header_entry& ct = i.get_header("Content-Type");
    ATF_CHECK_EQUAL(ct.value(), "text/X-test");
    ATF_CHECK(ct.has_attr("version"));
    ATF_CHECK_EQUAL(ct.get_attr("version"), "0");

    std::string line;

    i.getline(line);
    ATF_CHECK(i.good());
    ATF_CHECK_EQUAL(line, "Word");

    i.getline(line);
    ATF_CHECK(i.good());
    ATF_CHECK_EQUAL(line, "123");

    i.getline(line);
    ATF_CHECK(!i.good());
}

ATF_TEST_CASE(int_errors);
ATF_TEST_CASE_HEAD(int_errors)
{
    set("descr", "Tests the internalizer class with some erroneous data");
}
ATF_TEST_CASE_BODY(int_errors)
{
    using atf::serial::format_error;
    using atf::serial::internalizer;

    // Missing blank line after headers.
    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }

    // Missing content type.
    {
        std::string str;
        str += "Foo: bar\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }

    // Missing version.
    {
        std::string str;
        str += "Content-Type: text/X-test\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }

    // Multi-word header name.
    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "Foo bar: baz\n\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }

    // Out of order content type.
    {
        std::string str;
        str += "Foo: bar\n";
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }

    // Content type mismatch.
    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-foo", 0), format_error);
    }

    // Version mismatch.
    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 1), format_error);
    }

    // Missing attribute value.
    {
        std::string str;
        str += "Content-Type: text/X-test; version=\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }

    // Missing start quotes.
    {
        std::string str;
        str += "Content-Type: text/X-test; version=0\"\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }

    // Missing end quotes.
    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\n";
        str += "\n";

        std::istringstream ss(str);
        ATF_CHECK_THROW(internalizer(ss, "text/X-test", 0), format_error);
    }
}

ATF_TEST_CASE(int_lineno);
ATF_TEST_CASE_HEAD(int_lineno)
{
    set("descr", "Tests that the internalizer properlykeeps track of the "
                 "current line number");
}
ATF_TEST_CASE_BODY(int_lineno)
{
    using atf::serial::format_error;
    using atf::serial::internalizer;

    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n\n";

        std::istringstream ss(str);
        internalizer i(ss, "text/X-test", 0);
        ATF_CHECK_EQUAL(i.lineno(), 3);
    }

    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n";
        str += "Another-Heaader: foo-bar\n\n";

        std::istringstream ss(str);
        internalizer i(ss, "text/X-test", 0);
        ATF_CHECK_EQUAL(i.lineno(), 4);
    }

    {
        std::string str;
        str += "Content-Type: text/X-test; version=\"0\"\n\n";
        str += "foo\n";
        str += "bar\n";
        str += "baz";

        std::istringstream ss(str);
        internalizer i(ss, "text/X-test", 0);
        ATF_CHECK_EQUAL(i.lineno(), 3);

        std::string tmp;
        ATF_CHECK_EQUAL(i.getline(tmp).lineno(), 4);
        ATF_CHECK_EQUAL(i.getline(tmp).lineno(), 5);
        ATF_CHECK_EQUAL(i.getline(tmp).lineno(), 6);
        ATF_CHECK_EQUAL(i.getline(tmp).lineno(), 6);
    }
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add test cases for the "externalizer" class.
    tcs.push_back(&ext_empty);
    tcs.push_back(&ext_attrs);
    tcs.push_back(&ext_data);

    // Add test cases for the "internalizer" class.
    tcs.push_back(&int_empty);
    tcs.push_back(&int_attrs);
    tcs.push_back(&int_data);
    tcs.push_back(&int_errors);
    tcs.push_back(&int_lineno);
}
