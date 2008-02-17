//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007, 2008 The NetBSD Foundation, Inc.
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

extern "C" {
#include "atf-c/expand.h"
}

#include "atf/exceptions.hpp"
#include "atf/expand.hpp"
#include "atf/sanity.hpp"

namespace impl = atf::expand;
#define IMPL_NAME "atf::expand"

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

std::set< std::string >
impl::expand_glob(const std::string& glob,
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
impl::is_glob(const std::string& glob)
{
    return atf_expand_is_glob(glob.c_str());
}

bool
impl::matches_glob(const std::string& glob, const std::string& candidate)
{
    bool result;
    atf_error_t err;

    err = atf_expand_matches_glob(glob.c_str(), candidate.c_str(), &result);
    if (atf_is_error(err))
        throw_atf_error(err);

    return result;
}
