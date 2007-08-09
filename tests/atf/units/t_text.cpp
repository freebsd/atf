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

#include <set>
#include <vector>

#include <atf.hpp>

#include "atfprivate/text.hpp"

ATF_TEST_CASE(join);
ATF_TEST_CASE_HEAD(join)
{
    set("descr", "Tests the join function");
}
ATF_TEST_CASE_BODY(join)
{
    using atf::text::join;

    // First set of tests using a non-sorted collection, std::vector.
    {
        std::vector< std::string > words;
        std::string str;

        words.clear();
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, "");

        words.clear();
        words.push_back("");
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, "");

        words.clear();
        words.push_back("");
        words.push_back("");
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, ",");

        words.clear();
        words.push_back("foo");
        words.push_back("");
        words.push_back("baz");
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, "foo,,baz");

        words.clear();
        words.push_back("foo");
        words.push_back("bar");
        words.push_back("baz");
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, "foo,bar,baz");
    }

    // Second set of tests using a sorted collection, std::set.
    {
        std::set< std::string > words;
        std::string str;

        words.clear();
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, "");

        words.clear();
        words.insert("");
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, "");

        words.clear();
        words.insert("foo");
        words.insert("");
        words.insert("baz");
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, ",baz,foo");

        words.clear();
        words.insert("foo");
        words.insert("bar");
        words.insert("baz");
        str = join(words, ",");
        ATF_CHECK_EQUAL(str, "bar,baz,foo");
    }
}

ATF_TEST_CASE(split);
ATF_TEST_CASE_HEAD(split)
{
    set("descr", "Tests the split function");
}
ATF_TEST_CASE_BODY(split)
{
    using atf::text::split;

    std::vector< std::string > words;

    words = split("", " ");
    ATF_CHECK_EQUAL(words.size(), 0);

    words = split(" ", " ");
    ATF_CHECK_EQUAL(words.size(), 0);

    words = split("    ", " ");
    ATF_CHECK_EQUAL(words.size(), 0);

    words = split("a b", " ");
    ATF_CHECK_EQUAL(words.size(), 2);
    ATF_CHECK_EQUAL(words[0], "a");
    ATF_CHECK_EQUAL(words[1], "b");

    words = split("a b c d", " ");
    ATF_CHECK_EQUAL(words.size(), 4);
    ATF_CHECK_EQUAL(words[0], "a");
    ATF_CHECK_EQUAL(words[1], "b");
    ATF_CHECK_EQUAL(words[2], "c");
    ATF_CHECK_EQUAL(words[3], "d");

    words = split("foo bar", " ");
    ATF_CHECK_EQUAL(words.size(), 2);
    ATF_CHECK_EQUAL(words[0], "foo");
    ATF_CHECK_EQUAL(words[1], "bar");

    words = split("foo bar baz foobar", " ");
    ATF_CHECK_EQUAL(words.size(), 4);
    ATF_CHECK_EQUAL(words[0], "foo");
    ATF_CHECK_EQUAL(words[1], "bar");
    ATF_CHECK_EQUAL(words[2], "baz");
    ATF_CHECK_EQUAL(words[3], "foobar");

    words = split(" foo bar", " ");
    ATF_CHECK_EQUAL(words.size(), 2);
    ATF_CHECK_EQUAL(words[0], "foo");
    ATF_CHECK_EQUAL(words[1], "bar");

    words = split("foo  bar", " ");
    ATF_CHECK_EQUAL(words.size(), 2);
    ATF_CHECK_EQUAL(words[0], "foo");
    ATF_CHECK_EQUAL(words[1], "bar");

    words = split("foo bar ", " ");
    ATF_CHECK_EQUAL(words.size(), 2);
    ATF_CHECK_EQUAL(words[0], "foo");
    ATF_CHECK_EQUAL(words[1], "bar");

    words = split("  foo  bar  ", " ");
    ATF_CHECK_EQUAL(words.size(), 2);
    ATF_CHECK_EQUAL(words[0], "foo");
    ATF_CHECK_EQUAL(words[1], "bar");
}

ATF_TEST_CASE(split_delims);
ATF_TEST_CASE_HEAD(split_delims)
{
    set("descr", "Tests the split function using different delimiters");
}
ATF_TEST_CASE_BODY(split_delims)
{
    using atf::text::split;

    std::vector< std::string > words;

    words = split("", "/");
    ATF_CHECK_EQUAL(words.size(), 0);

    words = split(" ", "/");
    ATF_CHECK_EQUAL(words.size(), 1);
    ATF_CHECK_EQUAL(words[0], " ");

    words = split("    ", "/");
    ATF_CHECK_EQUAL(words.size(), 1);
    ATF_CHECK_EQUAL(words[0], "    ");

    words = split("a/b", "/");
    ATF_CHECK_EQUAL(words.size(), 2);
    ATF_CHECK_EQUAL(words[0], "a");
    ATF_CHECK_EQUAL(words[1], "b");

    words = split("aLONGDELIMbcdLONGDELIMef", "LONGDELIM");
    ATF_CHECK_EQUAL(words.size(), 3);
    ATF_CHECK_EQUAL(words[0], "a");
    ATF_CHECK_EQUAL(words[1], "bcd");
    ATF_CHECK_EQUAL(words[2], "ef");
}

ATF_TEST_CASE(to_string);
ATF_TEST_CASE_HEAD(to_string)
{
    set("descr", "Tests the to_string function");
}
ATF_TEST_CASE_BODY(to_string)
{
    using atf::text::to_string;

    ATF_CHECK_EQUAL(to_string('a'), "a");
    ATF_CHECK_EQUAL(to_string("a"), "a");
    ATF_CHECK_EQUAL(to_string(5), "5");
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&join);
    tcs.push_back(&split);
    tcs.push_back(&split_delims);
    tcs.push_back(&to_string);
}
