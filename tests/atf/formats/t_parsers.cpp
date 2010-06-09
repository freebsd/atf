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

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "atf-c++/formats.hpp"
#include "atf-c++/macros.hpp"
#include "atf-c++/parser.hpp"
#include "atf-c++/sanity.hpp"
#include "atf-c++/text.hpp"

typedef std::vector< std::string > string_vector;

class atffile_reader : protected atf::formats::atf_atffile_reader {
    void
    got_conf(const std::string& name, const std::string& val)
    {
        m_calls.push_back("got_conf(" + name + ", " + val + ")");
    }

    void
    got_prop(const std::string& name, const std::string& val)
    {
        m_calls.push_back("got_prop(" + name + ", " + val + ")");
    }

    void
    got_tp(const std::string& name, bool isglob)
    {
        m_calls.push_back("got_tp(" + name + ", " + (isglob ? "true" : "false")
                  + ")");
    }

    void
    got_eof(void)
    {
        m_calls.push_back("got_eof()");
    }

public:
    atffile_reader(std::istream& is) :
        atf::formats::atf_atffile_reader(is)
    {
    }

    void
    read(void)
    {
        atf_atffile_reader::read();
    }

    string_vector m_calls;
};

class config_reader : protected atf::formats::atf_config_reader {
    void
    got_var(const std::string& name, const std::string& val)
    {
        m_calls.push_back("got_var(" + name + ", " + val + ")");
    }

    void
    got_eof(void)
    {
        m_calls.push_back("got_eof()");
    }

public:
    config_reader(std::istream& is) :
        atf::formats::atf_config_reader(is)
    {
    }

    void
    read(void)
    {
        atf_config_reader::read();
    }

    string_vector m_calls;
};

class tp_reader : protected atf::formats::atf_tp_reader {
    void
    got_tc(const std::string& ident,
           const std::map< std::string, std::string >& md)
    {
        std::string call = "got_tc(" + ident + ", {";
        for (std::map< std::string, std::string >::const_iterator iter =
             md.begin(); iter != md.end(); iter++) {
            if (iter != md.begin())
                call += ", ";
            call += (*iter).first + '=' + (*iter).second;
        }
        call += "})";
        m_calls.push_back(call);
    }

    void
    got_eof(void)
    {
        m_calls.push_back("got_eof()");
    }

public:
    tp_reader(std::istream& is) :
        atf::formats::atf_tp_reader(is)
    {
    }

    void
    read(void)
    {
        atf_tp_reader::read();
    }

    string_vector m_calls;
};

class tps_reader : protected atf::formats::atf_tps_reader {
    void
    got_info(const std::string& what, const std::string& val)
    {
        m_calls.push_back("got_info(" + what + ", " + val + ")");
    }

    void
    got_ntps(size_t ntps)
    {
        m_calls.push_back("got_ntps(" + atf::text::to_string(ntps) + ")");
    }

    void
    got_tp_start(const std::string& tpname, size_t ntcs)
    {
        m_calls.push_back("got_tp_start(" + tpname + ", " +
                          atf::text::to_string(ntcs) + ")");
    }

    void
    got_tp_end(const std::string& reason)
    {
        m_calls.push_back("got_tp_end(" + reason + ")");
    }

    void
    got_tc_start(const std::string& tcname)
    {
        m_calls.push_back("got_tc_start(" + tcname + ")");
    }

    void
    got_tc_end(const atf::tests::tcr& tcr)
    {
        std::string r;
        if (tcr.get_state() == atf::tests::tcr::passed_state)
            r = "passed";
        else if (tcr.get_state() == atf::tests::tcr::failed_state)
            r = "failed, " + tcr.get_reason();
        else if (tcr.get_state() == atf::tests::tcr::skipped_state)
            r = "skipped, " + tcr.get_reason();
        else
            UNREACHABLE;

        m_calls.push_back("got_tc_end(" + r + ")");
    }

    void
    got_tc_stdout_line(const std::string& line)
    {
        m_calls.push_back("got_tc_stdout_line(" + line + ")");
    }

    void
    got_tc_stderr_line(const std::string& line)
    {
        m_calls.push_back("got_tc_stderr_line(" + line + ")");
    }

    void
    got_eof(void)
    {
        m_calls.push_back("got_eof()");
    }

public:
    tps_reader(std::istream& is) :
        atf::formats::atf_tps_reader(is)
    {
    }

    void
    read(void)
    {
        atf_tps_reader::read();
    }

    string_vector m_calls;
};

template< class Reader >
std::pair< string_vector, string_vector >
do_read(const char* input)
{
    string_vector errors;

    std::istringstream is(input);
    Reader reader(is);
    try {
        reader.read();
    } catch (const atf::parser::parse_errors& pes) {
        for (std::vector< atf::parser::parse_error >::const_iterator iter =
             pes.begin(); iter != pes.end(); iter++)
            errors.push_back(*iter);
    } catch (const atf::parser::parse_error& pe) {
        ATF_FAIL("Raised a lonely parse error: " +
                 atf::text::to_string(pe.first) + ": " + pe.second);
        UNREACHABLE;
    }

    return std::make_pair(reader.m_calls, errors);
}

static
void
check_equal(const char* expected[], const string_vector& actual)
{
    const char** expected_iter = expected;
    string_vector::const_iterator actual_iter = actual.begin();

    bool equals = true;
    while (equals && *expected_iter != NULL && actual_iter != actual.end()) {
        if (*expected_iter != *actual_iter) {
            equals = false;
        } else {
            expected_iter++;
            actual_iter++;
        }
    }
    if (equals && ((*expected_iter == NULL && actual_iter != actual.end()) ||
                   (*expected_iter != NULL && actual_iter == actual.end())))
        equals = false;

    if (!equals) {
        std::cerr << "EXPECTED:\n";
        for (expected_iter = expected; *expected_iter != NULL; expected_iter++)
            std::cerr << *expected_iter << "\n";

        std::cerr << "ACTUAL:\n";
        for (actual_iter = actual.begin(); actual_iter != actual.end();
             actual_iter++)
            std::cerr << *actual_iter << "\n";

        ATF_FAIL("Expected results differ to actual values");
    }
}

template< class Reader >
void
do_test(const char* input, const char* exp_calls[], const char* exp_errors[])
{
    const std::pair< string_vector, string_vector > actual =
        do_read< Reader >(input);
    check_equal(exp_calls, actual.first);
    check_equal(exp_errors, actual.second);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_1);
ATF_TEST_CASE_BODY(atffile_1)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
    ;

    const char* exp_calls[] = {
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_2);
ATF_TEST_CASE_BODY(atffile_2)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "# This is a comment on a line of its own.\n"
        "# And this is another one.\n"
        "\n"
        "	    # Another after some whitespace.\n"
        "\n"
        "# The last one after an empty line.\n"
    ;

    const char* exp_calls[] = {
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_3);
ATF_TEST_CASE_BODY(atffile_3)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "conf: var1=value1\n"
        "conf: var2 = value2\n"
        "conf: var3	=	value3\n"
        "conf: var4	    =	    value4\n"
        "\n"
        "conf:var5=value5\n"
        "    conf:var6=value6\n"
        "\n"
        "conf: var7 = \"This is a long value.\"\n"
        "conf: var8 = \"Single-word\"\n"
        "conf: var9 = \"    Single-word	\"\n"
        "conf: var10 = Single-word\n"
    ;

    const char* exp_calls[] = {
        "got_conf(var1, value1)",
        "got_conf(var2, value2)",
        "got_conf(var3, value3)",
        "got_conf(var4, value4)",
        "got_conf(var5, value5)",
        "got_conf(var6, value6)",
        "got_conf(var7, This is a long value.)",
        "got_conf(var8, Single-word)",
        "got_conf(var9,     Single-word	)",
        "got_conf(var10, Single-word)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_4);
ATF_TEST_CASE_BODY(atffile_4)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "prop: var1=value1\n"
        "prop: var2 = value2\n"
        "prop: var3	=	value3\n"
        "prop: var4	    =	    value4\n"
        "\n"
        "prop:var5=value5\n"
        "    prop:var6=value6\n"
        "\n"
        "prop: var7 = \"This is a long value.\"\n"
        "prop: var8 = \"Single-word\"\n"
        "prop: var9 = \"    Single-word	\"\n"
        "prop: var10 = Single-word\n"
    ;

    const char* exp_calls[] = {
        "got_prop(var1, value1)",
        "got_prop(var2, value2)",
        "got_prop(var3, value3)",
        "got_prop(var4, value4)",
        "got_prop(var5, value5)",
        "got_prop(var6, value6)",
        "got_prop(var7, This is a long value.)",
        "got_prop(var8, Single-word)",
        "got_prop(var9,     Single-word	)",
        "got_prop(var10, Single-word)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_5);
ATF_TEST_CASE_BODY(atffile_5)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "tp:foo\n"
        "tp: foo\n"
        "tp:  foo\n"
        "tp:	foo\n"
        "tp:	    foo\n"
        "tp: \"name with spaces\"\n"
        "tp: \"single-word\"\n"
        "tp: single-word\n"
        "\n"
        "tp-glob:foo*?bar\n"
        "tp-glob: foo*?bar\n"
        "tp-glob:  foo*?bar\n"
        "tp-glob:	foo*?bar\n"
        "tp-glob:	    foo*?bar\n"
        "tp-glob: \"glob * with ? spaces\"\n"
        "tp-glob: \"single-*-word\"\n"
        "tp-glob: single-*-word\n"
    ;

    const char* exp_calls[] = {
        "got_tp(foo, false)",
        "got_tp(foo, false)",
        "got_tp(foo, false)",
        "got_tp(foo, false)",
        "got_tp(foo, false)",
        "got_tp(name with spaces, false)",
        "got_tp(single-word, false)",
        "got_tp(single-word, false)",
        "got_tp(foo*?bar, true)",
        "got_tp(foo*?bar, true)",
        "got_tp(foo*?bar, true)",
        "got_tp(foo*?bar, true)",
        "got_tp(foo*?bar, true)",
        "got_tp(glob * with ? spaces, true)",
        "got_tp(single-*-word, true)",
        "got_tp(single-*-word, true)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_6);
ATF_TEST_CASE_BODY(atffile_6)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "prop: foo = bar # A comment.\n"
        "conf: foo = bar # A comment.\n"
        "tp: foo # A comment.\n"
        "tp-glob: foo # A comment.\n"
    ;

    const char* exp_calls[] = {
        "got_prop(foo, bar)",
        "got_conf(foo, bar)",
        "got_tp(foo, false)",
        "got_tp(foo, true)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_50);
ATF_TEST_CASE_BODY(atffile_50)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "foo\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `foo'; expected conf, #, prop, tp, tp-glob, a new line or eof",
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_51);
ATF_TEST_CASE_BODY(atffile_51)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "foo bar\n"
        "baz\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `foo'; expected conf, #, prop, tp, tp-glob, a new line or eof",
        "4: Unexpected token `baz'; expected conf, #, prop, tp, tp-glob, a new line or eof",
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_52);
ATF_TEST_CASE_BODY(atffile_52)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "conf\n"
        "conf:\n"
        "conf: foo =\n"
        "conf: bar = # A comment.\n"
        "\n"
        "prop\n"
        "prop:\n"
        "prop: foo =\n"
        "prop: bar = # A comment.\n"
        "\n"
        "tp\n"
        "tp:\n"
        "tp: # A comment.\n"
        "\n"
        "tp-glob\n"
        "tp-glob:\n"
        "tp-glob: # A comment.\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected `:'",
        "4: Unexpected token `<<NEWLINE>>'; expected variable name",
        "5: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        "6: Unexpected token `#'; expected word or quoted string",
        "8: Unexpected token `<<NEWLINE>>'; expected `:'",
        "9: Unexpected token `<<NEWLINE>>'; expected property name",
        "10: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        "11: Unexpected token `#'; expected word or quoted string",
        "13: Unexpected token `<<NEWLINE>>'; expected `:'",
        "14: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        "15: Unexpected token `#'; expected word or quoted string",
        "17: Unexpected token `<<NEWLINE>>'; expected `:'",
        "18: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        "19: Unexpected token `#'; expected word or quoted string",
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_53);
ATF_TEST_CASE_BODY(atffile_53)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "prop: foo = \"Correct value\" # With comment.\n"
        "\n"
        "prop: bar = # A comment.\n"
        "\n"
        "prop: baz = \"Last variable\"\n"
        "\n"
        "# End of file.\n"
    ;

    const char* exp_calls[] = {
        "got_prop(foo, Correct value)",
        NULL
    };

    const char* exp_errors[] = {
        "5: Unexpected token `#'; expected word or quoted string",
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(atffile_54);
ATF_TEST_CASE_BODY(atffile_54)
{
    const char* input =
        "Content-Type: application/X-atf-atffile; version=\"1\"\n"
        "\n"
        "prop: foo = \"\n"
        "prop: bar = \"text\n"
        "prop: baz = \"te\\\"xt\n"
        "prop: last = \"\\\"\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Missing double quotes before end of line",
        "4: Missing double quotes before end of line",
        "5: Missing double quotes before end of line",
        "6: Missing double quotes before end of line",
        NULL
    };

    do_test< atffile_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_1);
ATF_TEST_CASE_BODY(config_1)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
    ;

    const char* exp_calls[] = {
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_2);
ATF_TEST_CASE_BODY(config_2)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "# This is a comment on a line of its own.\n"
        "# And this is another one.\n"
        "\n"
        "	    # Another after some whitespace.\n"
        "\n"
        "# The last one after an empty line.\n"
    ;

    const char* exp_calls[] = {
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_3);
ATF_TEST_CASE_BODY(config_3)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "var1=value1\n"
        "var2 = value2\n"
        "var3	=	value3\n"
        "var4	    =	    value4\n"
        "\n"
        "var5=value5\n"
        "    var6=value6\n"
        "\n"
        "var7 = \"This is a long value.\"\n"
        "var8 = \"Single-word\"\n"
        "var9 = \"    Single-word	\"\n"
        "var10 = Single-word\n"
    ;

    const char* exp_calls[] = {
        "got_var(var1, value1)",
        "got_var(var2, value2)",
        "got_var(var3, value3)",
        "got_var(var4, value4)",
        "got_var(var5, value5)",
        "got_var(var6, value6)",
        "got_var(var7, This is a long value.)",
        "got_var(var8, Single-word)",
        "got_var(var9,     Single-word	)",
        "got_var(var10, Single-word)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_4);
ATF_TEST_CASE_BODY(config_4)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "foo = bar # A comment.\n"
    ;

    const char* exp_calls[] = {
        "got_var(foo, bar)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_50);
ATF_TEST_CASE_BODY(config_50)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "foo\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected equal sign",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_51);
ATF_TEST_CASE_BODY(config_51)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "foo bar\n"
        "baz\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `bar'; expected equal sign",
        "4: Unexpected token `<<NEWLINE>>'; expected equal sign",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_52);
ATF_TEST_CASE_BODY(config_52)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "foo =\n"
        "bar = # A comment.\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        "4: Unexpected token `#'; expected word or quoted string",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_53);
ATF_TEST_CASE_BODY(config_53)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "foo = \"Correct value\" # With comment.\n"
        "\n"
        "bar = # A comment.\n"
        "\n"
        "baz = \"Last variable\"\n"
        "\n"
        "# End of file.\n"
    ;

    const char* exp_calls[] = {
        "got_var(foo, Correct value)",
        NULL
    };

    const char* exp_errors[] = {
        "5: Unexpected token `#'; expected word or quoted string",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(config_54);
ATF_TEST_CASE_BODY(config_54)
{
    const char* input =
        "Content-Type: application/X-atf-config; version=\"1\"\n"
        "\n"
        "foo = \"\n"
        "bar = \"text\n"
        "baz = \"te\\\"xt\n"
        "last = \"\\\"\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Missing double quotes before end of line",
        "4: Missing double quotes before end of line",
        "5: Missing double quotes before end of line",
        "6: Missing double quotes before end of line",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_1);
ATF_TEST_CASE_BODY(headers_1)
{
    const char* input =
        ""
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Unexpected token `<<EOF>>'; expected a header name",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_2);
ATF_TEST_CASE_BODY(headers_2)
{
    const char* input =
        "Content-Type\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Unexpected token `<<NEWLINE>>'; expected `:'",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_3);
ATF_TEST_CASE_BODY(headers_3)
{
    const char* input =
        "Content-Type:\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Unexpected token `<<NEWLINE>>'; expected a textual value",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_4);
ATF_TEST_CASE_BODY(headers_4)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "2: Unexpected token `<<EOF>>'; expected a header name",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_5);
ATF_TEST_CASE_BODY(headers_5)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@;\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Unexpected token `<<NEWLINE>>'; expected an attribute name",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_6);
ATF_TEST_CASE_BODY(headers_6)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@; version\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Unexpected token `<<NEWLINE>>'; expected `='",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_7);
ATF_TEST_CASE_BODY(headers_7)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@; version=\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_8);
ATF_TEST_CASE_BODY(headers_8)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@; version=\"@CONTENT_VERSION@\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Missing double quotes before end of line",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_9);
ATF_TEST_CASE_BODY(headers_9)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@; version=@CONTENT_VERSION@\"\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "1: Missing double quotes before end of line",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_10);
ATF_TEST_CASE_BODY(headers_10)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@; version=@CONTENT_VERSION@\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "2: Unexpected token `<<EOF>>'; expected a header name",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_11);
ATF_TEST_CASE_BODY(headers_11)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@; version=\"@CONTENT_VERSION@\"\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "2: Unexpected token `<<EOF>>'; expected a header name",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(headers_12);
ATF_TEST_CASE_BODY(headers_12)
{
    const char* input =
        "Content-Type: @CONTENT_TYPE@; version=\"@CONTENT_VERSION@\"\n"
        "a b\n"
        "a-b:\n"
        "a-b: foo;\n"
        "a-b: foo; var\n"
        "a-b: foo; var=\n"
        "a-b: foo; var=\"a\n"
        "a-b: foo; var=a\"\n"
        "a-b: foo; var=\"a\";\n"
        "a-b: foo; var=\"a\"; second\n"
        "a-b: foo; var=\"a\"; second=\n"
        "a-b: foo; var=\"a\"; second=\"b\n"
        "a-b: foo; var=\"a\"; second=b\"\n"
        "a-b: foo; var=\"a\"; second=\"b\"\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "2: Unexpected token `b'; expected `:'",
        "3: Unexpected token `<<NEWLINE>>'; expected a textual value",
        "4: Unexpected token `<<NEWLINE>>'; expected an attribute name",
        "5: Unexpected token `<<NEWLINE>>'; expected `='",
        "6: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        "7: Missing double quotes before end of line",
        "8: Missing double quotes before end of line",
        "9: Unexpected token `<<NEWLINE>>'; expected an attribute name",
        "10: Unexpected token `<<NEWLINE>>'; expected `='",
        "11: Unexpected token `<<NEWLINE>>'; expected word or quoted string",
        "12: Missing double quotes before end of line",
        "13: Missing double quotes before end of line",
        NULL
    };

    do_test< config_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_1);
ATF_TEST_CASE_BODY(tp_1)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: test_case_1\n"
        "\n"
        "ident: test_case_2\n"
        "\n"
        "ident: test_case_3\n"
    ;

    const char* exp_calls[] = {
        "got_tc(test_case_1, {ident=test_case_1})",
        "got_tc(test_case_2, {ident=test_case_2})",
        "got_tc(test_case_3, {ident=test_case_3})",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_2);
ATF_TEST_CASE_BODY(tp_2)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: test_case_1\n"
        "descr: This is the description\n"
        "timeout: 300\n"
        "\n"
        "ident: test_case_2\n"
        "\n"
        "ident: test_case_3\n"
        "X-prop1: A custom property\n"
        "descr: Third test case\n"
    ;

    const char* exp_calls[] = {
        "got_tc(test_case_1, {descr=This is the description, ident=test_case_1, timeout=300})",
        "got_tc(test_case_2, {ident=test_case_2})",
        "got_tc(test_case_3, {X-prop1=A custom property, descr=Third test case, ident=test_case_3})",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_3);
ATF_TEST_CASE_BODY(tp_3)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: single_test\n"
        "descr: Some description\n"
        "timeout: 300\n"
        "require.arch: thearch\n"
        "require.config: foo-bar\n"
        "require.machine: themachine\n"
        "require.progs: /bin/cp mv\n"
        "require.user: root\n"
    ;

    const char* exp_calls[] = {
        "got_tc(single_test, {descr=Some description, ident=single_test, require.arch=thearch, require.config=foo-bar, require.machine=themachine, require.progs=/bin/cp mv, require.user=root, timeout=300})",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_4);
ATF_TEST_CASE_BODY(tp_4)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident:   single_test    \n"
        "descr:      Some description	\n"
    ;

    const char* exp_calls[] = {
        "got_tc(single_test, {descr=Some description, ident=single_test})",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_50);
ATF_TEST_CASE_BODY(tp_50)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<EOF>>'; expected property name",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_51);
ATF_TEST_CASE_BODY(tp_51)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "\n"
        "\n"
        "\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected property name",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_52);
ATF_TEST_CASE_BODY(tp_52)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: test1\n"
        "ident: test2\n"
    ;

    const char* exp_calls[] = {
        "got_tc(test1, {ident=test1})",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_53);
ATF_TEST_CASE_BODY(tp_53)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "descr: Out of order\n"
        "ident: test1\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: First property of a test case must be 'ident'",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_54);
ATF_TEST_CASE_BODY(tp_54)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident:\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: The value for 'ident' cannot be empty",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_55);
ATF_TEST_CASE_BODY(tp_55)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: +*,\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: The identifier must match ^[_A-Za-z0-9]+$; was '+*,'",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_56);
ATF_TEST_CASE_BODY(tp_56)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: test\n"
        "timeout: hello\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "4: The timeout property requires an integer value",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_57);
ATF_TEST_CASE_BODY(tp_57)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: test\n"
        "unknown: property\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "4: Unknown property 'unknown'",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_58);
ATF_TEST_CASE_BODY(tp_58)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "ident: test\n"
        "X-foo:\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "4: The value for 'X-foo' cannot be empty",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tp_59);
ATF_TEST_CASE_BODY(tp_59)
{
    const char* input =
        "Content-Type: application/X-atf-tp; version=\"1\"\n"
        "\n"
        "\n"
        "ident: test\n"
        "timeout: 300\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected property name",
        NULL
    };

    do_test< tp_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_1);
ATF_TEST_CASE_BODY(tps_1)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 0\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(0)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_2);
ATF_TEST_CASE_BODY(tps_2)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 2\n"
        "tp-start: first-prog, 0\n"
        "tp-end: first-prog\n"
        "tp-start: second-prog, 0\n"
        "tp-end: second-prog, This program failed\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(2)",
        "got_tp_start(first-prog, 0)",
        "got_tp_end()",
        "got_tp_start(second-prog, 0)",
        "got_tp_end(This program failed)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_3);
ATF_TEST_CASE_BODY(tps_3)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 2\n"
        "tp-start: first-prog, 3\n"
        "tc-start: first-test\n"
        "tc-end: first-test, passed\n"
        "tc-start: second-test\n"
        "tc-end: second-test, skipped, Testing skipped reason\n"
        "tc-start: third-test\n"
        "tc-end: third-test, failed, Testing failed reason\n"
        "tp-end: first-prog\n"
        "tp-start: second-prog, 3\n"
        "tc-start: first-test\n"
        "tc-so:first stdout line for 1st test\n"
        "tc-se:first stderr line for 1st test\n"
        "tc-so:second stdout line for 1st test\n"
        "tc-se:second stderr line for 1st test\n"
        "tc-end: first-test, passed\n"
        "tc-start: second-test\n"
        "tc-so:first stdout line for 2nd test\n"
        "tc-se:first stderr line for 2nd test\n"
        "tc-so:second stdout line for 2nd test\n"
        "tc-se:second stderr line for 2nd test\n"
        "tc-end: second-test, skipped, Testing skipped reason\n"
        "tc-start: third-test\n"
        "tc-so:first stdout line for 3rd test\n"
        "tc-se:first stderr line for 3rd test\n"
        "tc-so:second stdout line for 3rd test\n"
        "tc-se:second stderr line for 3rd test\n"
        "tc-end: third-test, failed, Testing failed reason\n"
        "tp-end: second-prog, This program failed\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(2)",
        "got_tp_start(first-prog, 3)",
        "got_tc_start(first-test)",
        "got_tc_end(passed)",
        "got_tc_start(second-test)",
        "got_tc_end(skipped, Testing skipped reason)",
        "got_tc_start(third-test)",
        "got_tc_end(failed, Testing failed reason)",
        "got_tp_end()",
        "got_tp_start(second-prog, 3)",
        "got_tc_start(first-test)",
        "got_tc_stdout_line(first stdout line for 1st test)",
        "got_tc_stderr_line(first stderr line for 1st test)",
        "got_tc_stdout_line(second stdout line for 1st test)",
        "got_tc_stderr_line(second stderr line for 1st test)",
        "got_tc_end(passed)",
        "got_tc_start(second-test)",
        "got_tc_stdout_line(first stdout line for 2nd test)",
        "got_tc_stderr_line(first stderr line for 2nd test)",
        "got_tc_stdout_line(second stdout line for 2nd test)",
        "got_tc_stderr_line(second stderr line for 2nd test)",
        "got_tc_end(skipped, Testing skipped reason)",
        "got_tc_start(third-test)",
        "got_tc_stdout_line(first stdout line for 3rd test)",
        "got_tc_stderr_line(first stderr line for 3rd test)",
        "got_tc_stdout_line(second stdout line for 3rd test)",
        "got_tc_stderr_line(second stderr line for 3rd test)",
        "got_tc_end(failed, Testing failed reason)",
        "got_tp_end(This program failed)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_4);
ATF_TEST_CASE_BODY(tps_4)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info: a, foo\n"
        "info: b, bar\n"
        "info: c, baz\n"
        "tps-count: 2\n"
        "tp-start: first-prog, 3\n"
        "tc-start: first-test\n"
        "tc-end: first-test, passed\n"
        "tc-start: second-test\n"
        "tc-end: second-test, skipped, Testing skipped reason\n"
        "tc-start: third-test\n"
        "tc-end: third-test, failed, Testing failed reason\n"
        "tp-end: first-prog\n"
        "tp-start: second-prog, 3\n"
        "tc-start: first-test\n"
        "tc-so:first stdout line for 1st test\n"
        "tc-se:first stderr line for 1st test\n"
        "tc-so:second stdout line for 1st test\n"
        "tc-se:second stderr line for 1st test\n"
        "tc-end: first-test, passed\n"
        "tc-start: second-test\n"
        "tc-so:first stdout line for 2nd test\n"
        "tc-se:first stderr line for 2nd test\n"
        "tc-so:second stdout line for 2nd test\n"
        "tc-se:second stderr line for 2nd test\n"
        "tc-end: second-test, skipped, Testing skipped reason\n"
        "tc-start: third-test\n"
        "tc-so:first stdout line for 3rd test\n"
        "tc-se:first stderr line for 3rd test\n"
        "tc-so:second stdout line for 3rd test\n"
        "tc-se:second stderr line for 3rd test\n"
        "tc-end: third-test, failed, Testing failed reason\n"
        "tp-end: second-prog, This program failed\n"
        "info: d, foo\n"
        "info: e, bar\n"
        "info: f, baz\n"
    ;

    const char* exp_calls[] = {
        "got_info(a, foo)",
        "got_info(b, bar)",
        "got_info(c, baz)",
        "got_ntps(2)",
        "got_tp_start(first-prog, 3)",
        "got_tc_start(first-test)",
        "got_tc_end(passed)",
        "got_tc_start(second-test)",
        "got_tc_end(skipped, Testing skipped reason)",
        "got_tc_start(third-test)",
        "got_tc_end(failed, Testing failed reason)",
        "got_tp_end()",
        "got_tp_start(second-prog, 3)",
        "got_tc_start(first-test)",
        "got_tc_stdout_line(first stdout line for 1st test)",
        "got_tc_stderr_line(first stderr line for 1st test)",
        "got_tc_stdout_line(second stdout line for 1st test)",
        "got_tc_stderr_line(second stderr line for 1st test)",
        "got_tc_end(passed)",
        "got_tc_start(second-test)",
        "got_tc_stdout_line(first stdout line for 2nd test)",
        "got_tc_stderr_line(first stderr line for 2nd test)",
        "got_tc_stdout_line(second stdout line for 2nd test)",
        "got_tc_stderr_line(second stderr line for 2nd test)",
        "got_tc_end(skipped, Testing skipped reason)",
        "got_tc_start(third-test)",
        "got_tc_stdout_line(first stdout line for 3rd test)",
        "got_tc_stderr_line(first stderr line for 3rd test)",
        "got_tc_stdout_line(second stdout line for 3rd test)",
        "got_tc_stderr_line(second stderr line for 3rd test)",
        "got_tc_end(failed, Testing failed reason)",
        "got_tp_end(This program failed)",
        "got_info(d, foo)",
        "got_info(e, bar)",
        "got_info(f, baz)",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_5);
ATF_TEST_CASE_BODY(tps_5)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 1\n"
        "tp-start: the-prog, 1\n"
        "tc-start: the-test\n"
        "tc-so:--- a	2007-11-04 14:00:41.000000000 +0100\n"
        "tc-so:+++ b	2007-11-04 14:00:48.000000000 +0100\n"
        "tc-so:@@ -1,7 +1,7 @@\n"
        "tc-so: This test is meant to simulate a diff.\n"
        "tc-so: Blank space at beginning of context lines must be preserved.\n"
        "tc-so: \n"
        "tc-so:-First original line.\n"
        "tc-so:-Second original line.\n"
        "tc-so:+First modified line.\n"
        "tc-so:+Second modified line.\n"
        "tc-so: \n"
        "tc-so: EOF\n"
        "tc-end: the-test, passed\n"
        "tp-end: the-prog\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(1)",
        "got_tp_start(the-prog, 1)",
        "got_tc_start(the-test)",
        "got_tc_stdout_line(--- a	2007-11-04 14:00:41.000000000 +0100)",
        "got_tc_stdout_line(+++ b	2007-11-04 14:00:48.000000000 +0100)",
        "got_tc_stdout_line(@@ -1,7 +1,7 @@)",
        "got_tc_stdout_line( This test is meant to simulate a diff.)",
        "got_tc_stdout_line( Blank space at beginning of context lines must be preserved.)",
        "got_tc_stdout_line( )",
        "got_tc_stdout_line(-First original line.)",
        "got_tc_stdout_line(-Second original line.)",
        "got_tc_stdout_line(+First modified line.)",
        "got_tc_stdout_line(+Second modified line.)",
        "got_tc_stdout_line( )",
        "got_tc_stdout_line( EOF)",
        "got_tc_end(passed)",
        "got_tp_end()",
        "got_eof()",
        NULL
    };

    const char* exp_errors[] = {
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_50);
ATF_TEST_CASE_BODY(tps_50)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "foo\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `foo'; expected tps-count or info field",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_51);
ATF_TEST_CASE_BODY(tps_51)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected `:'",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_52);
ATF_TEST_CASE_BODY(tps_52)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count:\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected number of test programs",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_53);
ATF_TEST_CASE_BODY(tps_53)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 1\n"
        "foo\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(1)",
        NULL
    };

    const char* exp_errors[] = {
        "4: Unexpected token `foo'; expected start of test program",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_54);
ATF_TEST_CASE_BODY(tps_54)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 1\n"
        "foo\n"
        "tp-start\n"
        "tp-start:\n"
        "tp-start: foo\n"
        "tp-start: foo,\n"
        "tp-start: foo, 0\n"
        "bar\n"
        "tp-start: foo, 0\n"
        "tp-end\n"
        "tp-start: foo, 0\n"
        "tp-end:\n"
        "tp-start: foo, 0\n"
        "tp-end: bar\n"
        "tp-start: foo, 0\n"
        "tp-end: foo,\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(1)",
        NULL
    };

    const char* exp_errors[] = {
        "4: Unexpected token `foo'; expected start of test program",
        "5: Unexpected token `<<NEWLINE>>'; expected `:'",
        "6: Unexpected token `<<NEWLINE>>'; expected test program name",
        "7: Unexpected token `<<NEWLINE>>'; expected `,'",
        "8: Unexpected token `<<NEWLINE>>'; expected number of test programs",
        "10: Unexpected token `bar'; expected end of test program",
        "12: Unexpected token `<<NEWLINE>>'; expected `:'",
        "14: Unexpected token `<<NEWLINE>>'; expected test program name",
        "16: Test program name used in terminator does not match opening",
        "18: Empty reason for failed test program",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_55);
ATF_TEST_CASE_BODY(tps_55)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 1\n"
        "tp-start: foo, 1\n"
        "foo\n"
        "tc-start\n"
        "tc-start:\n"
        "tc-start: foo\n"
        "bar\n"
        "tc-start: foo\n"
        "tc-end\n"
        "tc-start: foo\n"
        "tc-end:\n"
        "tc-start: foo\n"
        "tc-end: bar\n"
        "tc-start: foo\n"
        "tc-end: foo\n"
        "tc-start: foo\n"
        "tc-end: foo,\n"
        "tp-end: foo\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(1)",
        "got_tp_start(foo, 1)",
        NULL
    };

    const char* exp_errors[] = {
        "5: Unexpected token `foo'; expected start of test case",
        "6: Unexpected token `<<NEWLINE>>'; expected `:'",
        "7: Unexpected token `<<NEWLINE>>'; expected test case name",
        "9: Unexpected token `bar'; expected end of test case or test case's stdout/stderr line",
        "11: Unexpected token `<<NEWLINE>>'; expected `:'",
        "13: Unexpected token `<<NEWLINE>>'; expected test case name",
        "15: Test case name used in terminator does not match opening",
        "17: Unexpected token `<<NEWLINE>>'; expected `,'",
        "19: Unexpected token `<<NEWLINE>>'; expected passed, failed or skipped",
        "20: Unexpected token `tp-end'; expected start of test case",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_56);
ATF_TEST_CASE_BODY(tps_56)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 1\n"
        "tp-start: foo, 1\n"
        "tc-start: foo\n"
        "tc-end: foo, passe\n"
        "tc-start: foo\n"
        "tc-end: foo, passed,\n"
        "tc-start: bar\n"
        "tc-end: bar, failed\n"
        "tc-start: bar\n"
        "tc-end: bar, failed,\n"
        "tc-start: baz\n"
        "tc-end: baz, skipped\n"
        "tc-start: baz\n"
        "tc-end: baz, skipped,\n"
        "tp-end: foo\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(1)",
        "got_tp_start(foo, 1)",
        "got_tc_start(foo)",
        NULL
    };

    const char* exp_errors[] = {
        "6: Unexpected token `passe'; expected passed, failed or skipped",
        "8: Unexpected token `,'; expected new line",
        "10: Unexpected token `<<NEWLINE>>'; expected `,'",
        "12: Empty reason for failed test case result",
        "14: Unexpected token `<<NEWLINE>>'; expected `,'",
        "16: Empty reason for skipped test case result",
        "17: Unexpected token `tp-end'; expected start of test case",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_57);
ATF_TEST_CASE_BODY(tps_57)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 2\n"
        "tp-start: foo, 0\n"
        "tp-end: foo\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(2)",
        "got_tp_start(foo, 0)",
        "got_tp_end()",
        NULL
    };

    const char* exp_errors[] = {
        "6: Unexpected token `<<EOF>>'; expected start of test program",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_58);
ATF_TEST_CASE_BODY(tps_58)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "tps-count: 1\n"
        "tp-start: foo, 0\n"
        "tp-end: foo\n"
        "tp-start: bar, 0\n"
        "tp-end: bar\n"
    ;

    const char* exp_calls[] = {
        "got_ntps(1)",
        "got_tp_start(foo, 0)",
        "got_tp_end()",
        NULL
    };

    const char* exp_errors[] = {
        "6: Unexpected token `tp-start'; expected end of stream or info field",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_59);
ATF_TEST_CASE_BODY(tps_59)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected `:'",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_60);
ATF_TEST_CASE_BODY(tps_60)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info:\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected info property name",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_61);
ATF_TEST_CASE_BODY(tps_61)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info: a\n"
    ;

    const char* exp_calls[] = {
        NULL
    };

    const char* exp_errors[] = {
        "3: Unexpected token `<<NEWLINE>>'; expected `,'",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_62);
ATF_TEST_CASE_BODY(tps_62)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info: a,\n"
    ;

    const char* exp_calls[] = {
        "got_info(a, )",
        NULL
    };

    const char* exp_errors[] = {
        "4: Unexpected token `<<EOF>>'; expected tps-count or info field",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_63);
ATF_TEST_CASE_BODY(tps_63)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info: a, b\n"
    ;

    const char* exp_calls[] = {
        "got_info(a, b)",
        NULL
    };

    const char* exp_errors[] = {
        "4: Unexpected token `<<EOF>>'; expected tps-count or info field",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_64);
ATF_TEST_CASE_BODY(tps_64)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info: a, b\n"
        "tps-count\n"
    ;

    const char* exp_calls[] = {
        "got_info(a, b)",
        NULL
    };

    const char* exp_errors[] = {
        "4: Unexpected token `<<NEWLINE>>'; expected `:'",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_65);
ATF_TEST_CASE_BODY(tps_65)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info: a, b\n"
        "tps-count:\n"
    ;

    const char* exp_calls[] = {
        "got_info(a, b)",
        NULL
    };

    const char* exp_errors[] = {
        "4: Unexpected token `<<NEWLINE>>'; expected number of test programs",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_TEST_CASE_WITHOUT_HEAD(tps_66);
ATF_TEST_CASE_BODY(tps_66)
{
    const char* input =
        "Content-Type: application/X-atf-tps; version=\"2\"\n"
        "\n"
        "info: a, b\n"
        "tps-count: 0\n"
        "info\n"
    ;

    const char* exp_calls[] = {
        "got_info(a, b)",
        "got_ntps(0)",
        NULL
    };

    const char* exp_errors[] = {
        "5: Unexpected token `<<NEWLINE>>'; expected `:'",
        NULL
    };

    do_test< tps_reader >(input, exp_calls, exp_errors);
}

ATF_INIT_TEST_CASES(tcs)
{
    ATF_ADD_TEST_CASE(tcs, atffile_1);
    ATF_ADD_TEST_CASE(tcs, atffile_2);
    ATF_ADD_TEST_CASE(tcs, atffile_3);
    ATF_ADD_TEST_CASE(tcs, atffile_4);
    ATF_ADD_TEST_CASE(tcs, atffile_5);
    ATF_ADD_TEST_CASE(tcs, atffile_6);
    ATF_ADD_TEST_CASE(tcs, atffile_50);
    ATF_ADD_TEST_CASE(tcs, atffile_51);
    ATF_ADD_TEST_CASE(tcs, atffile_52);
    ATF_ADD_TEST_CASE(tcs, atffile_53);
    ATF_ADD_TEST_CASE(tcs, atffile_54);
    ATF_ADD_TEST_CASE(tcs, config_1);
    ATF_ADD_TEST_CASE(tcs, config_2);
    ATF_ADD_TEST_CASE(tcs, config_3);
    ATF_ADD_TEST_CASE(tcs, config_4);
    ATF_ADD_TEST_CASE(tcs, config_50);
    ATF_ADD_TEST_CASE(tcs, config_51);
    ATF_ADD_TEST_CASE(tcs, config_52);
    ATF_ADD_TEST_CASE(tcs, config_53);
    ATF_ADD_TEST_CASE(tcs, config_54);
    ATF_ADD_TEST_CASE(tcs, headers_1);
    ATF_ADD_TEST_CASE(tcs, headers_2);
    ATF_ADD_TEST_CASE(tcs, headers_3);
    ATF_ADD_TEST_CASE(tcs, headers_4);
    ATF_ADD_TEST_CASE(tcs, headers_5);
    ATF_ADD_TEST_CASE(tcs, headers_6);
    ATF_ADD_TEST_CASE(tcs, headers_7);
    ATF_ADD_TEST_CASE(tcs, headers_8);
    ATF_ADD_TEST_CASE(tcs, headers_9);
    ATF_ADD_TEST_CASE(tcs, headers_10);
    ATF_ADD_TEST_CASE(tcs, headers_11);
    ATF_ADD_TEST_CASE(tcs, headers_12);
    ATF_ADD_TEST_CASE(tcs, tp_1);
    ATF_ADD_TEST_CASE(tcs, tp_2);
    ATF_ADD_TEST_CASE(tcs, tp_3);
    ATF_ADD_TEST_CASE(tcs, tp_4);
    ATF_ADD_TEST_CASE(tcs, tp_50);
    ATF_ADD_TEST_CASE(tcs, tp_51);
    ATF_ADD_TEST_CASE(tcs, tp_52);
    ATF_ADD_TEST_CASE(tcs, tp_53);
    ATF_ADD_TEST_CASE(tcs, tp_54);
    ATF_ADD_TEST_CASE(tcs, tp_55);
    ATF_ADD_TEST_CASE(tcs, tp_56);
    ATF_ADD_TEST_CASE(tcs, tp_57);
    ATF_ADD_TEST_CASE(tcs, tp_58);
    ATF_ADD_TEST_CASE(tcs, tp_59);
    ATF_ADD_TEST_CASE(tcs, tps_1);
    ATF_ADD_TEST_CASE(tcs, tps_2);
    ATF_ADD_TEST_CASE(tcs, tps_3);
    ATF_ADD_TEST_CASE(tcs, tps_4);
    ATF_ADD_TEST_CASE(tcs, tps_5);
    ATF_ADD_TEST_CASE(tcs, tps_50);
    ATF_ADD_TEST_CASE(tcs, tps_51);
    ATF_ADD_TEST_CASE(tcs, tps_52);
    ATF_ADD_TEST_CASE(tcs, tps_53);
    ATF_ADD_TEST_CASE(tcs, tps_54);
    ATF_ADD_TEST_CASE(tcs, tps_55);
    ATF_ADD_TEST_CASE(tcs, tps_56);
    ATF_ADD_TEST_CASE(tcs, tps_57);
    ATF_ADD_TEST_CASE(tcs, tps_58);
    ATF_ADD_TEST_CASE(tcs, tps_59);
    ATF_ADD_TEST_CASE(tcs, tps_60);
    ATF_ADD_TEST_CASE(tcs, tps_61);
    ATF_ADD_TEST_CASE(tcs, tps_62);
    ATF_ADD_TEST_CASE(tcs, tps_63);
    ATF_ADD_TEST_CASE(tcs, tps_64);
    ATF_ADD_TEST_CASE(tcs, tps_65);
    ATF_ADD_TEST_CASE(tcs, tps_66);
}
