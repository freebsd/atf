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

#include <cassert>

#include "atfprivate/text.hpp"

#include "atf/exceptions.hpp"
#include "atf/test_case_result.hpp"

// ------------------------------------------------------------------------
// The "test_case_result" class.
// ------------------------------------------------------------------------

atf::test_case_result::test_case_result(void) :
    m_status(status_failed),
    m_reason("Uninitialized test case result")
{
}

atf::test_case_result::test_case_result(atf::test_case_result::status s,
                                        const std::string& r) :
    m_status(s),
    m_reason(r)
{
    assert(m_reason.find('\n') == std::string::npos);
}

atf::test_case_result
atf::test_case_result::passed(void)
{
    return test_case_result(status_passed, "");
}

atf::test_case_result
atf::test_case_result::skipped(const std::string& reason)
{
    return test_case_result(status_skipped, reason);
}

atf::test_case_result
atf::test_case_result::failed(const std::string& reason)
{
    return test_case_result(status_failed, reason);
}

atf::test_case_result::status
atf::test_case_result::get_status(void)
    const
{
    return m_status;
}

const std::string&
atf::test_case_result::get_reason(void)
    const
{
    return m_reason;
}
