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

#include "atfprivate/serial.hpp"
#include "atfprivate/text.hpp"

namespace impl = atf::serial;
#define IMPL_NAME "atf::serial"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

static
impl::header_entry
parse_value(const std::string& name, const std::string& str)
{
    using impl::format_error;

    std::string::size_type pos = str.find("; ");
    const std::string& value = str.substr(0, pos);

    impl::attrs_map attrs;
    if (pos != std::string::npos) {
        std::vector< std::string > ws =
            atf::text::split(str.substr(pos + 2), "; ");

        for (std::vector< std::string >::const_iterator iter = ws.begin();
             iter != ws.end(); iter++) {
            const std::string& attr = *iter;

            std::string::size_type pos2 = attr.find("=");

            if (pos2 == std::string::npos)
                throw format_error("Invalid attribute `" + attr + "' "
                                   "in header field `" + name + "'");

            if (pos2 == attr.length() - 1)
                throw format_error("Empty attribute `" + attr + "' "
                                   "in header field `" + name + "'");

            if (pos2 < attr.length() - 1 &&
                attr[pos2 + 1] == '"' && attr[attr.length() - 1] != '"')
                throw format_error("Incorrectly balanced quotes in "
                                   "attribute `" + attr + "' "
                                   "in header field `" + name + "'");

            if (attr[pos2 + 1] != '"' && attr[attr.length() - 1] == '"')
                throw format_error("Incorrectly balanced quotes in "
                                   "attribute `" + attr + "' "
                                   "in header field `" + name + "'");

            const std::string& attrname = attr.substr(0, pos2);
            const std::string& attrval = attr.substr(pos2 + 1);

            if (attr[pos2] != '"' && attr[attr.length() - 1] != '"')
                attrs[attrname] = attrval;
            else
                attrs[attrname] = attrval.substr(1, attrval.length() - 2);
        }
    }

    return impl::header_entry(name, value, attrs);
}

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

    assert(!m_headers.empty());
    m_os << m_headers["Content-Type"];
    for (std::map< std::string, header_entry >::const_iterator iter =
         m_headers.begin(); iter != m_headers.end(); iter++) {
        if ((*iter).first != "Content-Type")
            m_os << (*iter).second;
    }
    m_os << '\n';

    m_inited = true;
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

std::ostream&
impl::externalizer::get_stream(void)
{
    return m_os;
}

// ------------------------------------------------------------------------
// The "internalizer" class.
// ------------------------------------------------------------------------

impl::internalizer::internalizer(std::istream& is,
                                 const std::string& fmt,
                                 int version) :
    m_is(is)
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
    std::string line;
    bool first = true;

    header_entry he;
    while ((m_is >> he).good() && !he.name().empty()) {
        if (first && he.name() != "Content-Type")
            throw impl::format_error("Could not determine content type");
        else
            first = false;

        m_headers[he.name()] = he;
    }

    if (!m_is.good())
        throw format_error("Unexpected end of stream");
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

std::istream&
impl::internalizer::get_stream(void)
{
    return m_is;
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

std::istream&
operator>>(std::istream& is, impl::header_entry& he)
{
    if (!is.good())
        return is;

    std::string line;
    std::getline(is, line);

    if (line.empty())
        he = impl::header_entry();
    else {
        std::string::size_type pos = line.find(": ");
        if (pos == std::string::npos)
            throw impl::format_error("Invalid header entry: `" + line +
                                     "'");

        const std::string& name = line.substr(0, pos);
        const std::string& val = line.substr(pos + 2);
        he = parse_value(name, val);
    }

    return is;
}

std::ostream&
operator<<(std::ostream& os, const impl::header_entry& he)
{
    os << he.name() << ": " << he.value();
    impl::attrs_map as = he.attrs();
    for (impl::attrs_map::const_iterator iter = as.begin();
         iter != as.end(); iter++) {
        os << "; " << (*iter).first << "=\"" << (*iter).second << "\"";
    }
    os << "\n";

    return os;
}
