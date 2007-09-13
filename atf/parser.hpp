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
#include <utility>
#include <vector>

namespace atf {
    namespace serial {
        class internalizer;
    }
}

namespace atf {
namespace parser {

// ------------------------------------------------------------------------
// The "parse_error" class.
// ------------------------------------------------------------------------

class parse_error : public std::runtime_error,
                    public std::pair< size_t, std::string > {
    mutable std::string m_msg;

public:
    parse_error(size_t, std::string);
    ~parse_error(void) throw();

    const char* what(void) const throw();
};

// ------------------------------------------------------------------------
// The "parse_errors" class.
// ------------------------------------------------------------------------

class parse_errors : public std::runtime_error,
                     public std::vector< parse_error > {
    std::vector< parse_error > m_errors;
    mutable std::string m_msg;

public:
    parse_errors(void);
    ~parse_errors(void) throw();

    const char* what(void) const throw();
};

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
    bool m_inited;
    size_t m_line;
    T m_type;
    std::string m_text;

public:
    token(void);
    token(size_t, T, const std::string& = "");

    size_t lineno(void) const;
    T type(void) const;
    const std::string& text(void) const;

    operator bool(void) const;
    bool operator!(void) const;
};

template< class T >
token< T >::token(void) :
    m_inited(false)
{
}

template< class T >
token< T >::token(size_t p_line, T p_type, const std::string& p_text) :
    m_inited(true),
    m_line(p_line),
    m_type(p_type),
    m_text(p_text)
{
}

template< class T >
size_t
token< T >::lineno(void)
    const
{
    return m_line;
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

template< class T >
token< T >::operator bool(void)
    const
{
    return m_inited;
}

template< class T >
bool
token< T >::operator!(void)
    const
{
    return !m_inited;
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
    size_t m_lineno;
    token< T > m_la;

    bool m_skipws;
    T m_eoftype, m_nltype, m_texttype;

    std::map< char, T > m_delims_map;
    std::string m_delims_str;

    char m_quotech;
    T m_quotetype;

    std::map< std::string, T > m_keywords_map;

    template< class TKZ >
    friend
    class parser;

public:
    typedef T token_type;

    tokenizer(IS&, bool, T, T, T, size_t = 1);

    size_t lineno(void) const;

    void add_delim(char, T);
    void add_keyword(const std::string&, T);
    void add_quote(char, T);

    token< T > next(void);
    std::string rest_of_line(void);
};

template< class T, class IS >
tokenizer< T, IS >::tokenizer(IS& is, bool skipws, T eoftype, T nltype,
                              T texttype, size_t p_lineno) :
    m_is(is),
    m_lineno(p_lineno),
    m_skipws(skipws),
    m_eoftype(eoftype),
    m_nltype(nltype),
    m_texttype(texttype),
    m_quotech(-1)
{
}

template< class T, class IS >
size_t
tokenizer< T, IS >::lineno(void)
    const
{
    return m_lineno;
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
void
tokenizer< T, IS >::add_quote(char ch, T type)
{
    m_quotech = ch;
    m_quotetype = type;
}

template< class T, class IS >
token< T >
tokenizer< T, IS >::next(void)
{
    if (m_la) {
        token< T > t = m_la;
        m_la = token< T >();
        if (t.type() == m_nltype)
            m_lineno++;
        return t;
    }

    char ch;
    std::string text;

    bool done = false, quoted = false;
    token< T > t(m_lineno, m_eoftype, "<<EOF>>");
    while (!done && m_is.get(ch).good()) {
        if (ch == m_quotech) {
            if (text.empty()) {
                bool escaped = false;
                while (!done && m_is.get(ch).good()) {
                    if (!escaped) {
                        if (ch == '\\')
                            escaped = true;
                        else if (ch == '\n') {
                            m_la = token< T >(m_lineno, m_nltype,
                                              "<<NEWLINE>>");
                            throw parse_error(t.lineno(),
                                              "Missing double quotes before "
                                              "end of line");
                        } else if (ch == m_quotech)
                            done = true;
                        else
                            text += ch;
                    } else {
                        text += ch;
                        escaped = false;
                    }
                }
                if (!m_is.good())
                    throw parse_error(t.lineno(),
                                      "Missing double quotes before "
                                      "end of file");
                t = token< T >(m_lineno, m_texttype, text);
                quoted = true;
            } else {
                m_is.unget();
                done = true;
            }
        } else {
            typename std::map< char, T >::const_iterator idelim;
            idelim = m_delims_map.find(ch);
            if (idelim != m_delims_map.end()) {
                done = true;
                if (text.empty())
                    t = token< T >(m_lineno, (*idelim).second,
                                   std::string("") + ch);
                else
                    m_is.unget();
            } else if (ch == '\n') {
                done = true;
                if (text.empty())
                    t = token< T >(m_lineno, m_nltype, "<<NEWLINE>>");
                else
                    m_is.unget();
            } else if (m_skipws && (ch == ' ' || ch == '\t')) {
                if (!text.empty())
                    done = true;
            } else
                text += ch;
        }
    }

    if (!quoted && !text.empty()) {
        typename std::map< std::string, T >::const_iterator ikw;
        ikw = m_keywords_map.find(text);
        if (ikw != m_keywords_map.end())
            t = token< T >(m_lineno, (*ikw).second, text);
        else
            t = token< T >(m_lineno, m_texttype, text);
    }

    if (t.type() == m_nltype)
        m_lineno++;

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
// The "parser" class.
// ------------------------------------------------------------------------

template< class TKZ >
class parser {
    TKZ& m_tkz;
    token< typename TKZ::token_type > m_last;
    parse_errors m_errors;
    bool m_thrown;

public:
    parser(TKZ& tkz) :
        m_tkz(tkz),
        m_thrown(false)
    {
    }

    ~parser(void)
    {
        if (!m_errors.empty() && !m_thrown)
            throw m_errors;
    }

    bool
    good(void)
        const
    {
        return m_tkz.m_is.good();
    }

    void
    add_error(const parse_error& pe)
    {
        m_errors.push_back(pe);
    }

    bool
    has_errors(void)
        const
    {
        return !m_errors.empty();
    }

    token< typename TKZ::token_type >
    next(void)
    {
        token< typename TKZ::token_type > t = m_tkz.next();

        m_last = t;

        if (t.type() == m_tkz.m_eoftype) {
            if (!m_errors.empty()) {
                m_thrown = true;
                throw m_errors;
            }
        }

        return t;
    }

    token< typename TKZ::token_type >
    expect(const typename TKZ::token_type& t1,
           const std::string& textual)
    {
        token< typename TKZ::token_type > t = next();

        if (t.type() != t1)
            throw parse_error(t.lineno(),
                              "Unexpected token `" + t.text() +
                              "'; expected " + textual);

        return t;
    }

    token< typename TKZ::token_type >
    expect(const typename TKZ::token_type& t1,
           const typename TKZ::token_type& t2,
           const std::string& textual)
    {
        token< typename TKZ::token_type > t = next();

        if (t.type() != t1 && t.type() != t2)
            throw parse_error(t.lineno(),
                              "Unexpected token `" + t.text() +
                              "'; expected " + textual);

        return t;
    }

    token< typename TKZ::token_type >
    expect(const typename TKZ::token_type& t1,
           const typename TKZ::token_type& t2,
           const typename TKZ::token_type& t3,
           const std::string& textual)
    {
        token< typename TKZ::token_type > t = next();

        if (t.type() != t1 && t.type() != t2 && t.type() != t3)
            throw parse_error(t.lineno(),
                              "Unexpected token `" + t.text() +
                              "'; expected " + textual);

        return t;
    }

    token< typename TKZ::token_type >
    expect(const typename TKZ::token_type& t1,
           const typename TKZ::token_type& t2,
           const typename TKZ::token_type& t3,
           const typename TKZ::token_type& t4,
           const typename TKZ::token_type& t5,
           const typename TKZ::token_type& t6,
           const typename TKZ::token_type& t7,
           const std::string& textual)
    {
        token< typename TKZ::token_type > t = next();

        if (t.type() != t1 && t.type() != t2 && t.type() != t3 &&
            t.type() != t4 && t.type() != t5 && t.type() != t6 &&
            t.type() != t7)
            throw parse_error(t.lineno(),
                              "Unexpected token `" + t.text() +
                              "'; expected " + textual);

        return t;
    }

    std::string
    rest_of_line(void)
    {
        return m_tkz.rest_of_line();
    }

    token< typename TKZ::token_type >
    reset(const typename TKZ::token_type& stop)
    {
        token< typename TKZ::token_type > t = m_last;

        while (t.type() != m_tkz.m_eoftype && t.type() != stop)
            t = next();

        return t;
    }
};

} // namespace parser
} // namespace atf

#endif // !defined(_ATF_PARSER_HPP_)
