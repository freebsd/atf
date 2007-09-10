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
#include "atf/parser.hpp"

// ------------------------------------------------------------------------
// Tests for the "token" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(token_getters);
ATF_TEST_CASE_HEAD(token_getters)
{
    set("descr", "Tests the token getters");
}
ATF_TEST_CASE_BODY(token_getters)
{
    using atf::parser::token;

    {
        token< int > t(0);
        ATF_CHECK_EQUAL(t.type(), 0);
        ATF_CHECK(t.text().empty());
    }

    {
        token< int > t(0, "foo");
        ATF_CHECK_EQUAL(t.type(), 0);
        ATF_CHECK_EQUAL(t.text(), "foo");
    }

    {
        token< int > t(1);
        ATF_CHECK_EQUAL(t.type(), 1);
        ATF_CHECK(t.text().empty());
    }

    {
        token< int > t(1, "bar");
        ATF_CHECK_EQUAL(t.type(), 1);
        ATF_CHECK_EQUAL(t.text(), "bar");
    }
}

// ------------------------------------------------------------------------
// Tests for the "tokenizer" class.
// ------------------------------------------------------------------------

#define EXPECT(tkz, enumtype, ttype, ttext) \
    do { \
        atf::parser::token< enumtype > t = tkz.next(); \
        ATF_CHECK(t.type() == ttype); \
        ATF_CHECK_EQUAL(t.text(), ttext); \
    } while (false);

namespace minimal {

    enum tokens {
        eof,
        nl,
        word,
    };

    class tokenizer : public atf::parser::tokenizer< tokens,
                                                     std::istream > {
    public:
        tokenizer(std::istream& is, bool skipws) :
            atf::parser::tokenizer< tokens, std::istream >
                (is, skipws, eof, nl, word)
        {
        }
    };

}

namespace delims {

    enum tokens {
        eof,
        nl,
        word,
        plus,
        minus,
        equal,
    };

    class tokenizer : public atf::parser::tokenizer< tokens,
                                                     std::istream > {
    public:
        tokenizer(std::istream& is, bool skipws) :
            atf::parser::tokenizer< tokens, std::istream >
                (is, skipws, eof, nl, word)
        {
            add_delim('+', plus);
            add_delim('-', minus);
            add_delim('=', equal);
        }
    };

}

namespace keywords {

    enum tokens {
        eof,
        nl,
        word,
        var,
        loop,
        endloop,
    };

    class tokenizer : public atf::parser::tokenizer< tokens,
                                                     std::istream > {
    public:
        tokenizer(std::istream& is, bool skipws) :
            atf::parser::tokenizer< tokens, std::istream >
                (is, skipws, eof, nl, word)
        {
            add_keyword("var", var);
            add_keyword("loop", loop);
            add_keyword("endloop", endloop);
        }
    };

}

ATF_TEST_CASE(tokenizer_minimal_nows);
ATF_TEST_CASE_HEAD(tokenizer_minimal_nows)
{
    set("descr", "Tests the tokenizer class using a minimal parser and "
                 "not skipping whitespace");
}
ATF_TEST_CASE_BODY(tokenizer_minimal_nows)
{
    using namespace minimal;

    {
        std::istringstream iss("");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("\n");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("\n\n\n");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "line 1");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1\n");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "line 1");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1\nline 2");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "line 1");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line 2");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1\nline 2\nline 3\n");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "line 1");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line 2");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line 3");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }
}

ATF_TEST_CASE(tokenizer_minimal_ws);
ATF_TEST_CASE_HEAD(tokenizer_minimal_ws)
{
    set("descr", "Tests the tokenizer class using a minimal parser and "
                 "skipping whitespace");
}
ATF_TEST_CASE_BODY(tokenizer_minimal_ws)
{
    using namespace minimal;

    {
        std::istringstream iss("");
        minimal::tokenizer mt(iss, true);

        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss(" \t ");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("\n");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss(" \t \n \t ");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("\n\n\n");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "1");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("   \tline\t   1\t");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "1");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1\n");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "1");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1\nline 2");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "1");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "2");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("line 1\nline 2\nline 3\n");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "1");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "2");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "3");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss(" \t line \t 1\n\tline\t2\n line 3 \n");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "1");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "2");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, word, "line");
        EXPECT(mt, tokens, word, "3");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }
}

ATF_TEST_CASE(tokenizer_delims_nows);
ATF_TEST_CASE_HEAD(tokenizer_delims_nows)
{
    set("descr", "Tests the tokenizer class using a parser with some "
                 "additional delimiters and not skipping whitespace");
}
ATF_TEST_CASE_BODY(tokenizer_delims_nows)
{
    using namespace delims;

    {
        std::istringstream iss("+-=");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, minus, "-");
        EXPECT(mt, tokens, equal, "=");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("+++");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("\n+\n++\n");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("foo+bar=baz");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "foo");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, word, "bar");
        EXPECT(mt, tokens, equal, "=");
        EXPECT(mt, tokens, word, "baz");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss(" foo\t+\tbar = baz ");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, " foo\t");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, word, "\tbar ");
        EXPECT(mt, tokens, equal, "=");
        EXPECT(mt, tokens, word, " baz ");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }
}

ATF_TEST_CASE(tokenizer_delims_ws);
ATF_TEST_CASE_HEAD(tokenizer_delims_ws)
{
    set("descr", "Tests the tokenizer class using a parser with some "
                 "additional delimiters and skipping whitespace");
}
ATF_TEST_CASE_BODY(tokenizer_delims_ws)
{
    using namespace delims;

    {
        std::istringstream iss(" foo\t+\tbar = baz ");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, word, "foo");
        EXPECT(mt, tokens, plus, "+");
        EXPECT(mt, tokens, word, "bar");
        EXPECT(mt, tokens, equal, "=");
        EXPECT(mt, tokens, word, "baz");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }
}

ATF_TEST_CASE(tokenizer_keywords_nows);
ATF_TEST_CASE_HEAD(tokenizer_keywords_nows)
{
    set("descr", "Tests the tokenizer class using a parser with some "
                 "additional keywords and not skipping whitespace");
}
ATF_TEST_CASE_BODY(tokenizer_keywords_nows)
{
    using namespace keywords;

    {
        std::istringstream iss("var");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, var, "var");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("va");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "va");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("vara");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "vara");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("var ");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, word, "var ");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("var\nloop\nendloop");
        tokenizer mt(iss, false);

        EXPECT(mt, tokens, var, "var");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, loop, "loop");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, endloop, "endloop");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }
}

ATF_TEST_CASE(tokenizer_keywords_ws);
ATF_TEST_CASE_HEAD(tokenizer_keywords_ws)
{
    set("descr", "Tests the tokenizer class using a parser with some "
                 "additional keywords and not skipping whitespace");
}
ATF_TEST_CASE_BODY(tokenizer_keywords_ws)
{
    using namespace keywords;

    {
        std::istringstream iss("var ");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, var, "var");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss(" var \n\tloop\t\n \tendloop \t");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, var, "var");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, loop, "loop");
        EXPECT(mt, tokens, nl, "<<NEWLINE>>");
        EXPECT(mt, tokens, endloop, "endloop");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }

    {
        std::istringstream iss("var loop endloop");
        tokenizer mt(iss, true);

        EXPECT(mt, tokens, var, "var");
        EXPECT(mt, tokens, loop, "loop");
        EXPECT(mt, tokens, endloop, "endloop");
        EXPECT(mt, tokens, eof, "<<EOF>>");
        EXPECT(mt, tokens, eof, "<<EOF>>");
    }
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add test cases for the "token" class.
    tcs.push_back(&token_getters);

    // Add test cases for the "tokenizer" class.
    tcs.push_back(&tokenizer_minimal_nows);
    tcs.push_back(&tokenizer_minimal_ws);
    tcs.push_back(&tokenizer_delims_nows);
    tcs.push_back(&tokenizer_delims_ws);
    tcs.push_back(&tokenizer_keywords_nows);
    tcs.push_back(&tokenizer_keywords_ws);
}
