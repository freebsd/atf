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

#if !defined(_ATF_TEST_CASE_RESULT_HPP_)
#define _ATF_TEST_CASE_RESULT_HPP_

#include <istream>
#include <ostream>
#include <string>

namespace atf {

// ------------------------------------------------------------------------
// The "test_case_result" class.
// ------------------------------------------------------------------------

//!
//! \brief Holds the results of a test case's execution.
//!
//! The test_case_result class holds the information that describes the
//! results of a test case's execution.  This is composed of an exit code
//! and a reason for that exit code.
//!
//! TODO: Complete documentation for this class.  Not done yet because it
//! is worth to investigate if this class could be rewritten as several
//! different classes, one for each status.
//!
class test_case_result {
public:
    enum status { status_passed, status_skipped, status_failed };

    test_case_result(void);

    static test_case_result passed(void);
    static test_case_result skipped(const std::string&);
    static test_case_result failed(const std::string&);

    status get_status(void) const;
    const std::string& get_reason(void) const;

private:
    status m_status;
    std::string m_reason;

    test_case_result(status, const std::string&);
};

} // namespace atf

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

std::ostream& operator<<(std::ostream&, const atf::test_case_result&);
std::istream& operator>>(std::istream&, atf::test_case_result&);

#endif // !defined(_ATF_TEST_CASE_RESULT_HPP_)
