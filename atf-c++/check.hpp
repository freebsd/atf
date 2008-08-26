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

#if !defined(_ATF_CXX_CHECK_HPP_)
#define _ATF_CXX_CHECK_HPP_

#include <string>

#include "atf-c++/fs.hpp"

extern "C" {
#include "atf-c/check.h"
}

namespace atf {
namespace check {

// ------------------------------------------------------------------------
// The "check_result" class.
// ------------------------------------------------------------------------

//!
//! \brief A class that contains results of executed command.
//!
//! The check_result class holds information about results
//! of executing arbitrary command and manages files containing
//! its output.
//!
class check_result {
    //!
    //! \brief Internal representation of a result.
    //!
    atf_check_result_t m_result;

    //!
    //! \brief Constructs an uninitialized results object, which must be
    //! filled in by exec immediately afterwards.
    //!
    check_result(void);

    friend check_result exec(char * const *);

public:
    //!
    //! \brief Destroys object and removes all managed files.
    //!
    ~check_result();

    //!
    //! \brief Returns command's exit status.
    //!
    int status() const;

    //!
    //! \brief Returns the path to file contaning command's stdout.
    //!
    const atf::fs::path stdout_path() const;

    //!
    //! \brief Returns the path to file contaning command's stderr.
    //!
    const atf::fs::path stderr_path() const;
};

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

check_result exec(char * const *);

} // namespace check
} // namespace atf

#endif // !defined(_ATF_CXX_CHECK_HPP_)
