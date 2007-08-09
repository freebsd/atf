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

#if !defined(_ATF_SERIAL_HPP_)
#define _ATF_SERIAL_HPP_

#include <istream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>

namespace atf {
namespace serial {

// ------------------------------------------------------------------------
// The "format_error" class.
// ------------------------------------------------------------------------

//!
//! \brief A class to signal format errors in external data formats.
//!
//! This error class is used to signal format errors while parsing some
//! externalized representation of a data structure.
//!
class format_error : public std::runtime_error {
public:
    format_error(const std::string&);
};

// ------------------------------------------------------------------------
// The "header_entry" class.
// ------------------------------------------------------------------------

typedef std::map< std::string, std::string > attrs_map;

class header_entry {
    std::string m_name;
    std::string m_value;
    attrs_map m_attrs;

public:
    header_entry(void);
    header_entry(const std::string&, const std::string&,
                 attrs_map = attrs_map());

    const std::string& name(void) const;
    const std::string& value(void) const;
    const attrs_map& attrs(void) const;
    bool has_attr(const std::string&) const;
    const std::string& get_attr(const std::string&) const;
};

// ------------------------------------------------------------------------
// The "externalizer" class.
// ------------------------------------------------------------------------

class externalizer {
    std::ostream& m_os;
    bool m_inited;
    std::map< std::string, header_entry > m_headers;

    void write_headers(void);

public:
    externalizer(std::ostream&, const std::string&, int);

    void add_header(const header_entry&);
    void flush(void);
    std::ostream& get_stream(void);

    template< class T >
    std::ostream& operator<<(const T&);
};

template< class T >
std::ostream&
externalizer::operator<<(const T& t)
{
    if (!m_inited)
        write_headers();

    return (m_os << t);
}

// ------------------------------------------------------------------------
// The "internalizer" class.
// ------------------------------------------------------------------------

class internalizer {
    std::istream& m_is;
    std::map< std::string, header_entry > m_headers;

    void read_headers(void);

public:
    internalizer(std::istream&, const std::string&, int);

    bool has_header(const std::string&) const;
    const header_entry& get_header(const std::string&) const;
    std::istream& get_stream(void);

    template< class T >
    std::istream& operator>>(T&);
};

template< class T >
std::istream&
internalizer::operator>>(T& t)
{
    return (m_is >> t);
}

} // namespace serial
} // namespace atf

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

std::istream& operator>>(std::istream&, atf::serial::header_entry&);
std::ostream& operator<<(std::ostream&, const atf::serial::header_entry&);

#endif // !defined(_ATF_SERIAL_HPP_)
