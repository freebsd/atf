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

#if !defined(_ATF_TEST_CASE_HPP_)
#define _ATF_TEST_CASE_HPP_

#include <map>
#include <sstream>
#include <utility>

#include <atf/test_case_result.hpp>

namespace atf {

class test_case {
    typedef std::map< std::string, std::string > variables_map;

    std::string m_ident;
    variables_map m_meta_data;

    std::string m_srcdir;
    std::string m_workdirbase;

    void ensure_boolean(const std::string&);
    void ensure_not_empty(const std::string&);

    void parse_props(void) const;

protected:
    virtual void head(void) = 0;
    virtual void body(void) const = 0;

public:
    test_case(const std::string&);
    virtual ~test_case(void);

    const std::string& get(const std::string&) const;
    bool get_bool(const std::string&) const;
    bool has(const std::string&) const;
    void set(const std::string&, const std::string&);

    const std::string& get_srcdir(void) const;

    void init(const std::string&, const std::string&);
    test_case_result run(void) const;
};

typedef std::pair< std::string, test_case_result > tcname_tcr;

} // namespace atf

std::ostream& operator<<(std::ostream&, const atf::tcname_tcr&);
std::istream& operator>>(std::istream&, atf::tcname_tcr&);

#define ATF_TEST_CASE(name) \
    class name : public atf::test_case { \
        void head(void); \
        void body(void) const; \
    public: \
        name(void) : atf::test_case(#name) {} \
    }; \
    static name name;

#define ATF_TEST_CASE_HEAD(name) \
    void \
    name::head(void)

#define ATF_TEST_CASE_BODY(name) \
    void \
    name::body(void) \
        const

#define ATF_FAIL(reason) \
    throw atf::test_case_result::failed(reason)

#define ATF_SKIP(reason) \
    throw atf::test_case_result::skipped(reason)

#define ATF_PASS() \
    throw atf::test_case_result::passed()

#define ATF_CHECK(x) \
    if (!(x)) { \
        std::ostringstream __atf_ss; \
        __atf_ss << "Line " << __LINE__ << ": " << #x << " not met"; \
        throw atf::test_case_result::failed(__atf_ss.str()); \
    }

#define ATF_CHECK_EQUAL(x, y) \
    if ((x) != (y)) { \
        std::ostringstream __atf_ss; \
        __atf_ss << "Line " << __LINE__ << ": " << #x << " != " << #y \
                 << " (" << (x) << " != " << (y) << ")"; \
        throw atf::test_case_result::failed(__atf_ss.str()); \
    }

#define ATF_CHECK_THROW(x, e) \
    try { \
        x; \
        ATF_FAIL(#x " did not throw " #e " as expected"); \
    } catch (const e& __atf_eo) { \
    }

#endif // !defined(_ATF_TEST_CASE_HPP_)
