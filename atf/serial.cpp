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

#include <cassert>

#include "atf/serial.hpp"
#include "atf/parser.hpp"
#include "atf/text.hpp"

namespace impl = atf::serial;
#define IMPL_NAME "atf::serial"

// ------------------------------------------------------------------------
// The quoted_string parser.
// ------------------------------------------------------------------------

namespace quoted_string {

enum tokens {
    eof,
    nl,
    text,
    quote,
    escape,
};

typedef atf::parser::token< tokens > token;

class tokenizer : public atf::parser::tokenizer< tokens, impl::internalizer > {
public:
    tokenizer(impl::internalizer& is, char quotech) :
        atf::parser::tokenizer< tokens, impl::internalizer >
            (is, false, eof, nl, text)
    {
        add_delim('\\', escape);
        add_delim(quotech, quote);
    }
};

//
// Reads a quoted string.  The stream is supposed to be placed just after
// the initial quote character, provided as the quotech parameter.  All
// text after it until the matching closing quote is returned.  Quotes can
// be embedded inside the text by prefixing them with the escape character.
//
static
std::string
read(impl::internalizer& is, char quotech)
{
    tokenizer tkz(is, quotech);

    std::string str;

    bool done = false;
    token tprev = token(eof);
    token t = tkz.next();
    while (!done) {
        switch (t.type()) {
        case nl:
            throw impl::format_error("Missing double quotes before end "
                                     "of line");

        case eof:
            throw impl::format_error("Missing double quotes before end "
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
// The header tokenizer.
// ------------------------------------------------------------------------

namespace header {

enum tokens {
    eof,
    nl,
    text,
    colon,
    semicolon,
    dblquote,
    equal
};

typedef atf::parser::token< tokens > token;

class tokenizer : public atf::parser::tokenizer< tokens, impl::internalizer > {
public:
    tokenizer(impl::internalizer& is) :
        atf::parser::tokenizer< tokens, impl::internalizer >
            (is, true, eof, nl, text)
    {
        add_delim(';', semicolon);
        add_delim(':', colon);
        add_delim('"', dblquote);
        add_delim('=', equal);
    }
};

inline
void
expect(token t, tokens type, const std::string& textual)
{
    if (t.type() != type)
        throw impl::format_error("Unexpected token `" + t.text() + "'; "
                                 "expected " + textual);
}

inline
void
expect(token t, tokens type1, tokens type2, const std::string& textual)
{
    if (t.type() != type1 && t.type() != type2)
        throw impl::format_error("Unexpected token `" + t.text() + "'; "
                                 "expected " + textual);
}

static
impl::internalizer&
read(impl::internalizer& is, impl::header_entry& he)
{
    using header::expect;

    header::tokenizer tkz(is);

    header::token t = tkz.next();
    if (t.type() == header::nl) {
        he = impl::header_entry();
        return is;
    }
    expect(t, header::text, "a header name");
    std::string hdr_name = t.text();

    t = tkz.next();
    expect(t, header::colon, "`:'");

    t = tkz.next();
    expect(t, header::text, "a textual value");
    std::string hdr_value = t.text();

    impl::attrs_map attrs;

    t = tkz.next();
    while (t.type() != header::nl) {
        expect(t, header::semicolon, "`;'");

        t = tkz.next();
        expect(t, header::text, "an attribute name");
        std::string attr_name = t.text();

        t = tkz.next();
        expect(t, header::equal, "`='");

        t = tkz.next();
        expect(t, header::text, header::dblquote, "an attribute value");
        if (t.type() == header::text) {
            std::string attr_value = t.text();
            attrs[attr_name] = attr_value;
        } else {
            assert(t.type() == header::dblquote);
            std::string attr_value = quoted_string::read(is, '"');
            attrs[attr_name] = attr_value;
        }

        t = tkz.next();
    }

    expect(t, header::nl, "end of header (blank line)");

    he = impl::header_entry(hdr_name, hdr_value, attrs);

    return is;
}

static
impl::externalizer&
write(impl::externalizer& os, const impl::header_entry& he)
{
    std::string line = he.name() + ": " + he.value();
    impl::attrs_map as = he.attrs();
    for (impl::attrs_map::const_iterator iter = as.begin();
         iter != as.end(); iter++) {
        assert((*iter).second.find('\"') == std::string::npos);
        line += "; " + (*iter).first + "=\"" + (*iter).second + "\"";
    }

    os.putline(line);

    return os;
}

} // namespace header

// ------------------------------------------------------------------------
// The "format_error" class.
// ------------------------------------------------------------------------

impl::format_error::format_error(const std::string& w) :
    std::runtime_error(w.c_str())
{
}

// ------------------------------------------------------------------------
// The "header_entry" class.
// ------------------------------------------------------------------------

impl::header_entry::header_entry(void)
{
}

impl::header_entry::header_entry(const std::string& n,
                                 const std::string& v,
                                 attrs_map as) :
    m_name(n),
    m_value(v),
    m_attrs(as)
{
}

const std::string&
impl::header_entry::name(void)
    const
{
    return m_name;
}

const std::string&
impl::header_entry::value(void)
    const
{
    return m_value;
}

const impl::attrs_map&
impl::header_entry::attrs(void)
    const
{
    return m_attrs;
}

bool
impl::header_entry::has_attr(const std::string& n)
    const
{
    return m_attrs.find(n) != m_attrs.end();
}

const std::string&
impl::header_entry::get_attr(const std::string& n)
    const
{
    attrs_map::const_iterator iter = m_attrs.find(n);
    assert(iter != m_attrs.end());
    return (*iter).second;
}

// ------------------------------------------------------------------------
// The "externalizer" class.
// ------------------------------------------------------------------------

impl::externalizer::externalizer(std::ostream& os,
                                 const std::string& fmt,
                                 int version) :
    m_os(os),
    m_inited(false)
{
    attrs_map attrs;
    attrs["version"] = text::to_string(version);
    m_headers["Content-Type"] = header_entry("Content-Type", fmt, attrs);
}

void
impl::externalizer::write_headers(void)
{
    assert(!m_inited);
    m_inited = true;

    assert(!m_headers.empty());
    header::write(*this, m_headers["Content-Type"]);
    for (std::map< std::string, header_entry >::const_iterator iter =
         m_headers.begin(); iter != m_headers.end(); iter++) {
        if ((*iter).first != "Content-Type")
            header::write(*this, (*iter).second);
    }
    putline("");
}

void
impl::externalizer::add_header(const header_entry& he)
{
    assert(!m_inited);
    m_headers[he.name()] = he;
}

void
impl::externalizer::flush(void)
{
    if (!m_inited)
        write_headers();

    m_os.flush();
}

// ------------------------------------------------------------------------
// The "internalizer" class.
// ------------------------------------------------------------------------

impl::internalizer::internalizer(std::istream& is,
                                 const std::string& fmt,
                                 int version) :
    m_is(is),
    m_lineno(1)
{
    read_headers();

    if (!has_header("Content-Type"))
        throw format_error("Could not determine content type");

    const header_entry& ct = get_header("Content-Type");
    if (ct.value() != fmt)
        throw format_error("Mismatched content type: expected `" + fmt +
                           "' but got `" + ct.value() + "'");

    if (!ct.has_attr("version"))
        throw format_error("Could not determine version");
    else if (ct.get_attr("version") != text::to_string(version))
        throw format_error("Mismatched version: expected `" +
                           text::to_string(version) + "' but got `" +
                           ct.get_attr("version") + "'");
}

void
impl::internalizer::read_headers(void)
{
    bool first = true;

    //
    // Grammar
    //
    // header = entry+ nl
    // entry = line nl
    // line = text colon text
    //        (semicolon (text equal (text | dblquote string dblquote)))*
    // string = quoted_string
    //

    header_entry he;
    while (header::read(*this, he).good() && !he.name().empty()) {
        if (first && he.name() != "Content-Type")
            throw impl::format_error("Could not determine content type");
        else
            first = false;

        m_headers[he.name()] = he;
    }

    if (!good())
        throw format_error("Unexpected end of stream");
}

size_t
impl::internalizer::lineno(void)
    const
{
    return m_lineno;
}

bool
impl::internalizer::has_header(const std::string& name)
    const
{
    return m_headers.find(name) != m_headers.end();
}

const impl::header_entry&
impl::internalizer::get_header(const std::string& name)
    const
{
    std::map< std::string, header_entry >::const_iterator iter =
        m_headers.find(name);
    assert(iter != m_headers.end());
    return (*iter).second;
}

impl::internalizer&
impl::internalizer::getline(std::string& str)
{
    if (m_is.good()) {
        m_lineno++;
        (void)std::getline(m_is, str);
    }
    return *this;
}

impl::internalizer&
impl::internalizer::get(char& ch)
{
    if (m_is.get(ch).good() && ch == '\n')
        m_lineno++;
    return *this;
}

impl::internalizer&
impl::internalizer::unget(void)
{
    m_is.unget();
    if (m_is.peek() == '\n')
        m_lineno--;
    return *this;
}


int
impl::internalizer::peek(void)
    const
{
    return m_is.peek();
}

bool
impl::internalizer::good(void)
    const
{
    return m_is.good();
}
