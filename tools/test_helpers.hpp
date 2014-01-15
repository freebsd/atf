//
// Automated Testing Framework (atf)
//
// Copyright (c) 2009 The NetBSD Foundation, Inc.
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

#if defined(UTILS_TEST_HELPERS_H)
#   error "Cannot include test_helpers.hpp more than once."
#else
#   define UTILS_TEST_HELPERS_H
#endif

#include <sstream>
#include <utility>

#include "atf-c++/detail/text.hpp"
#include "atf-c++/macros.hpp"

#include "parser.hpp"

namespace test_helpers_detail {

typedef std::vector< std::string > string_vector;

template< class Reader >
std::pair< string_vector, string_vector >
do_read(const char* input)
{
    string_vector errors;

    std::istringstream is(input);
    Reader reader(is);
    try {
        reader.read();
    } catch (const atf::parser::parse_errors& pes) {
        for (std::vector< atf::parser::parse_error >::const_iterator iter =
             pes.begin(); iter != pes.end(); iter++)
            errors.push_back(*iter);
    } catch (const atf::parser::parse_error& pe) {
        ATF_FAIL("Raised a lonely parse error: " +
                 atf::text::to_string(pe.first) + ": " + pe.second);
    }

    return std::make_pair(reader.m_calls, errors);
}

void check_equal(const char*[], const string_vector&);

} // namespace test_helpers_detail

template< class Reader >
void
do_parser_test(const char* input, const char* exp_calls[],
               const char* exp_errors[])
{
    const std::pair< test_helpers_detail::string_vector,
                     test_helpers_detail::string_vector >
        actual = test_helpers_detail::do_read< Reader >(input);
    test_helpers_detail::check_equal(exp_calls, actual.first);
    test_helpers_detail::check_equal(exp_errors, actual.second);
}
