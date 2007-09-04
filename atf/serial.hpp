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

#if !defined(_ATF_SERIAL_HPP_)
#define _ATF_SERIAL_HPP_

#include <istream>
#include <map>
#include <ostream>
#include <sstream>
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

    template< class T >
    externalizer& putline(const T&);
};

template< class T >
externalizer&
externalizer::putline(const T& l)
{
    if (!m_inited)
        write_headers();

    std::ostringstream ss;
    ss << l << '\n';
    m_os << ss.str();
    return *this;
}

// ------------------------------------------------------------------------
// The "internalizer" class.
// ------------------------------------------------------------------------

class internalizer {
    std::istream& m_is;
    size_t m_lineno;

    std::map< std::string, header_entry > m_headers;

    void read_headers(void);

public:
    internalizer(std::istream&, const std::string&, int);

    size_t lineno(void) const;

    bool has_header(const std::string&) const;
    const header_entry& get_header(const std::string&) const;

    internalizer& getline(std::string&);

    int get(void);
    internalizer& get(char&);
    internalizer& unget(void);
    int peek(void) const;
    bool good(void) const;
};

} // namespace serial
} // namespace atf

#endif // !defined(_ATF_SERIAL_HPP_)
