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

public:
    tokenizer(IS&, bool, T, T, T);

    void add_delim(char, T);
    void add_keyword(const std::string&, T);

    token< T > next(void);
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

} // namespace parser
} // namespace atf

#endif // !defined(_ATF_PARSER_HPP_)
