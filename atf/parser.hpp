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

#if !defined(_ATF_PARSER_HPP_)
#define _ATF_PARSER_HPP_

#include <map>
#include <stdexcept>
#include <string>

namespace atf {
namespace parser {

// ------------------------------------------------------------------------
// The "token" class.
// ------------------------------------------------------------------------

//!
//! \brief Representation of a read token.
//!
//! A pair that contains the information of a token read from a stream.
//! It contains the token's type and its associated data, if any.
//!
template< class T >
struct token {
    T m_type;
    std::string m_text;

public:
    token(T, const std::string& = "");

    T type(void) const;
    const std::string& text(void) const;
};

template< class T >
token< T >::token(T p_type, const std::string& p_text) :
    m_type(p_type),
    m_text(p_text)
{
}

template< class T >
T
token< T >::type(void)
    const
{
    return m_type;
}

template< class T >
const std::string&
token< T >::text(void)
    const
{
    return m_text;
}

// ------------------------------------------------------------------------
// The "tokenizer" class.
// ------------------------------------------------------------------------

//!
//! \brief A stream tokenizer.
//!
//! This template implements an extremely simple, line-oriented stream
//! tokenizer.  It is only able to recognize one character-long delimiters,
//! random-length keywords, skip whitespace and, anything that does not
//! match these rules is supposed to be a word.
//!
//! Parameter T: The token's type.  Typically an enum.
//! Parameter IS: The input stream's type.
//!
template< class T, class IS >
class tokenizer {
    IS& m_is;

    bool m_skipws;
    T m_eoftype, m_nltype, m_texttype;

    std::map< char, T > m_delims_map;
    std::string m_delims_str;

    std::map< std::string, T > m_keywords_map;

    template< class TKZ, class T2 >
    friend
    std::string
    read_literal(TKZ&, const T2&, char);

public:
    tokenizer(IS&, bool, T, T, T);

    void add_delim(char, T);
    void add_keyword(const std::string&, T);

    token< T > next(void);
    std::string rest_of_line(void);
};

template< class T, class IS >
tokenizer< T, IS >::tokenizer(IS& is, bool skipws, T eoftype, T nltype,
                              T texttype) :
    m_is(is),
    m_skipws(skipws),
    m_eoftype(eoftype),
    m_nltype(nltype),
    m_texttype(texttype)
{
}

template< class T, class IS >
void
tokenizer< T, IS >::add_delim(char delim, T type)
{
    m_delims_map[delim] = type;
    m_delims_str += delim;
}

template< class T, class IS >
void
tokenizer< T, IS >::add_keyword(const std::string& keyword, T type)
{
    m_keywords_map[keyword] = type;
}

template< class T, class IS >
token< T >
tokenizer< T, IS >::next(void)
{
    char ch;
    std::string text;

    bool done = false;
    token< T > t(m_eoftype, "<<EOF>>");
    while (!done && m_is.get(ch).good()) {
        typename std::map< char, T >::const_iterator idelim;
        idelim = m_delims_map.find(ch);
        if (idelim != m_delims_map.end()) {
            done = true;
            if (text.empty())
                t = token< T >((*idelim).second, std::string("") + ch);
            else
                m_is.unget();
        } else if (ch == '\n') {
            done = true;
            if (text.empty())
                t = token< T >(m_nltype, "<<NEWLINE>>");
            else
                m_is.unget();
        } else if (m_skipws && (ch == ' ' || ch == '\t')) {
            if (!text.empty())
                done = true;
        } else
            text += ch;
    }

    if (!text.empty()) {
        typename std::map< std::string, T >::const_iterator ikw;
        ikw = m_keywords_map.find(text);
        if (ikw != m_keywords_map.end())
            t = token< T >((*ikw).second, text);
        else
            t = token< T >(m_texttype, text);
    }

    return t;
}

template< class T, class IS >
std::string
tokenizer< T, IS >::rest_of_line(void)
{
    std::string str;
    while (m_is.good() && m_is.peek() != '\n')
        str += m_is.get();
    return str;
}

// ------------------------------------------------------------------------
// The quoted_string private parser.
// ------------------------------------------------------------------------

//
// IMPORTANT: Do not use this parser directly.  Call it through the
// read_literal free function defined below.
//

namespace quoted_string {

enum tokens {
    eof,
    nl,
    text,
    quote,
    escape,
};

typedef atf::parser::token< tokens > token;

template< class IS >
class tokenizer : public atf::parser::tokenizer< tokens, IS > {
public:
    tokenizer(IS& is, char quotech) :
        atf::parser::tokenizer< tokens, IS >(is, false, eof, nl, text)
    {
        atf::parser::tokenizer< tokens, IS >::add_delim('\\', escape);
        atf::parser::tokenizer< tokens, IS >::add_delim(quotech, quote);
    }
};

//
// Reads a quoted string.  The stream is supposed to be placed just after
// the initial quote character, provided as the quotech parameter.  All
// text after it until the matching closing quote is returned.  Quotes can
// be embedded inside the text by prefixing them with the escape character.
//
template< class IS >
std::string
read(IS& is, char quotech)
{
    using namespace quoted_string;

    tokenizer< IS > tkz(is, quotech);

    std::string str;

    bool done = false;
    token tprev = token(eof);
    token t = tkz.next();
    while (!done) {
        switch (t.type()) {
        case nl:
            throw std::runtime_error("Missing double quotes before end "
                                     "of line");

        case eof:
            throw std::runtime_error("Missing double quotes before end "
                                     "of file");

        case quote:
            if (tprev.type() == escape)
                str += quotech;
            else
                done = true;
            break;

        case escape:
            if (tprev.type() == escape)
                str += '\\';
            break;

        default:
            assert(t.type() == text);
            str += t.text();
        }

        if (!done) {
            tprev = t;
            t = tkz.next();
        }
    }

    return str;
}

} // namespace quoted_string

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

template< class TKZ, class T >
std::string
read_literal(TKZ& tkz, const T& quotetype, char quotechar)
{
    atf::parser::token< T > t = tkz.next();
    if (t.type() == tkz.m_texttype)
        return t.text();
    else if (t.type() == quotetype)
        return quoted_string::read(tkz.m_is, quotechar);
    else
        throw std::runtime_error(std::string("Expected word or quoted "
                                             "string (") +
                                 quotechar + ")");
}

} // namespace parser
} // namespace atf

#endif // !defined(_ATF_PARSER_HPP_)
