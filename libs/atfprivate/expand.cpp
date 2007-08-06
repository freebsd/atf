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

extern "C" {
#include <regex.h>

// REG_BASIC is just a synonym for 0, provided as a counterpart to
// REG_EXTENDED to improve readability.  It is not provided by all systems.
#if !defined(REG_BASIC)
#define REG_BASIC 0
#endif // !defined(REG_BASIC)
}

#include <cassert>

#include "atfprivate/exceptions.hpp"
#include "atfprivate/expand.hpp"

//
// Auxiliary function that converts a glob pattern into a regular
// expression ready to be processed by ::regcomp.  It is currently very
// limited and does not handle errors; those are handled when compiling
// the regular expression.
//
static
std::string
glob_to_regex(const std::string& glob)
{
    std::string regex;

    regex = "^";
    for (std::string::const_iterator iter = glob.begin();
         iter != glob.end(); iter++) {
        // NOTE: Keep this in sync with is_glob!
        if (*iter == '*')
            regex += ".*";
        else if (*iter == '?')
            regex += ".";
        else
            regex += *iter;
    }
    regex += "$";

    return regex;
}

//
// Auxiliary function that constructs and throws a pattern_error object
// based on the error code returned by one of ::regcomp or ::regexec and
// their corresponding ::regex_t object.
//
static inline
void
throw_pattern_error(int errcode, const regex_t* preg)
{
    // Calculate the length of the error message by using ::regerror with
    // a very small buffer.
    char lenbuf[1];
    size_t len = ::regerror(errcode, preg, lenbuf, 1);
    assert(len > 1);

    // Allocate a big-enough buffer to hold the complete error message and
    // throw an exception containing it.
    char* buf = new char[len];
    size_t len2 = ::regerror(errcode, preg, buf, len);
    assert(len == len2);
    throw atf::pattern_error(buf);
}

std::set< std::string >
atf::expand_glob(const std::string& glob,
                 const std::set< std::string >& candidates)
{
    std::set< std::string > exps;

    for (std::set< std::string >::const_iterator iter = candidates.begin();
         iter != candidates.end(); iter++)
        if (matches_glob(glob, *iter))
            exps.insert(*iter);

    return exps;
}

bool
atf::is_glob(const std::string& glob)
{
    // NOTE: Keep this in sync with glob_to_regex!
    return (glob.find('*') != std::string::npos) ||
           (glob.find('?') != std::string::npos);
}

bool
atf::matches_glob(const std::string& glob, const std::string& candidate)
{
    int res;
    ::regex_t preg;

    // Special case: regcomp does not like empty patterns.
    if (glob.empty())
        return candidate.empty();

    // Convert the glob pattern into a regular expression and compile it.
    std::string regex = glob_to_regex(glob);
    res = ::regcomp(&preg, regex.c_str(), REG_BASIC);
    if (res != 0)
        throw_pattern_error(res, &preg);

    // Check if the regular expression matches the candidate.
    res = ::regexec(&preg, candidate.c_str(), 0, NULL, 0);
    if (res != 0 && res != REG_NOMATCH)
        throw_pattern_error(res, &preg);
    return res == 0;
}
