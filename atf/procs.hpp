//
// Automated Testing Framework (atf)
//
// Copyright (c) 2008 The NetBSD Foundation, Inc.
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

#if !defined(_ATF_PROCS_HPP_)
#define _ATF_PROCS_HPP_

extern "C" {
#include <unistd.h>
}

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <atf/utils.hpp>

namespace atf {
namespace procs {

typedef std::set< pid_t > pid_set;

// ------------------------------------------------------------------------
// The "pid_grabber" class.
// ------------------------------------------------------------------------

class pid_grabber : public atf::utils::noncopyable {
    void* m_cookie;

public:
    pid_grabber(void);
    ~pid_grabber(void);

    bool can_get_children_of(void) const;
    pid_set get_children_of(pid_t);
};

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

typedef std::pair< pid_t, std::string > pid_error_pair;
typedef std::vector< pid_error_pair > errors_vector;

errors_vector kill_tree(pid_t, int, pid_grabber&);

} // namespace procs
} // namespace atf

#endif // !defined(_ATF_PROCS_HPP_)
