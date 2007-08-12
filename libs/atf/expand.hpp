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

#if !defined(_ATF_EXPAND_HPP_)
#define _ATF_EXPAND_HPP_

#include <set>
#include <stdexcept>
#include <string>

namespace atf {
namespace expand {

// ------------------------------------------------------------------------
// The "pattern_error" class.
// ------------------------------------------------------------------------

//!
//! \brief An error class to hold information about bad patterns.
//!
//! The pattern_error class is used to represent an error when parsing
//! or comparing a pattern against a string.
//!
//! TODO Mark non-copyable.
//!
class pattern_error : public std::runtime_error {
    //!
    //! \brief A pointer to the error message.
    //!
    //! This variable holds a pointer to the error message describing
    //! why the pattern failed.  This is a pointer to dynamic memory
    //! allocated by the code that constructed this class.
    //!
    char *m_what;

public:
    //!
    //! \brief Constructs a new pattern_error.
    //!
    //! Constructs a new pattern error, to be thrown as an exception,
    //! with the given error message.  The error message must be a
    //! pointer to dynamically allocated memory, obtained by using the
    //! 'new char[...]' construction.
    //!
    pattern_error(char *);

    //!
    //! \brief Destroys the pattern_error.
    //!
    //! Destroys the object and releases the memory that was held by the
    //! error message given during construction.
    //!
    ~pattern_error(void) throw();
};

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

//!
//! \brief Expands a glob pattern among multiple candidates.
//!
//! Given a glob pattern and a set of candidate strings, checks which of
//! those strings match the glob pattern and returns them.
//!
std::set< std::string > expand_glob(const std::string&,
                                    const std::set< std::string >&);

//!
//! \brief Checks if the given string is a glob pattern.
//!
//! Returns true if the given string is a glob pattern; i.e. if it contains
//! any character that will be expanded by expand_glob.
//!
bool is_glob(const std::string&);

//!
//! \brief Checks if a given string matches a glob pattern.
//!
//! Given a glob pattern and a string, checks whether the former matches
//! the latter.  Returns a boolean indicating this condition.
//!
bool matches_glob(const std::string&, const std::string&);

} // namespace expand
} // namespace atf

#endif // !defined(_ATF_EXPAND_HPP_)
