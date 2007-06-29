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

#ifndef _ATF_LIBATF_TEST_CASE_HPP_
#define _ATF_LIBATF_TEST_CASE_HPP_

#include <sstream>

#include <libatf/test_case_result.hpp>
#include <libatf/variables.hpp>

namespace atf {

class test_case {
    std::string m_ident;
    detail::variables_map m_meta_data;

protected:
    virtual void head(void) = 0;
    virtual void body(void) const = 0;

    void set(const std::string&, const std::string&);

public:
    test_case(const std::string&);
    virtual ~test_case(void);

    const std::string& get(const std::string&) const;

    void init(void);
    test_case_result run(void) const;
};

} // namespace atf

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

#define ATF_CHECK_EQUAL(x, y) \
    if ((x) != (y)) { \
        std::ostringstream ss; \
        ss << #x << " != " << #y << " (" << (x) << " != " << (y) << ")"; \
        throw atf::test_case_result::failed(__LINE__, ss.str()); \
    }

#endif // _ATF_LIBATF_TEST_CASE_HPP_
