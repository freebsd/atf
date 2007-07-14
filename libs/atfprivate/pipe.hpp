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

#if !defined(_ATF_PIPE_HPP_)
#define _ATF_PIPE_HPP_

//!
//! \file atf/pipe.hpp
//!
//! Includes the declaration of the pipe class.  This file is for
//! internal usage only and must not be included by the library user.
//!

#include <atfprivate/file_handle.hpp>

namespace atf {

//!
//! \brief Simple RAII model for anonymous pipes.
//!
//! The pipe class is a simple RAII model for anonymous pipes.  It
//! provides a portable constructor that allocates a new %pipe and creates
//! a pipe object that owns the two file handles associated to it: the
//! read end and the write end.
//!
//! These handles can be retrieved for modification according to
//! file_handle semantics.  Optionally, their ownership can be transferred
//! to external \a file_handle objects which comes handy when the two
//! ends need to be used in different places (i.e. after a POSIX fork()
//! system call).
//!
//! Pipes can be copied following the same semantics as file handles.
//! In other words, copying a %pipe object invalidates the source one.
//!
//! \see file_handle
//!
class pipe
{
    //!
    //! \brief The %pipe's read end file handle.
    //!
    file_handle m_read_end;

    //!
    //! \brief The %pipe's write end file handle.
    //!
    file_handle m_write_end;

public:
    //!
    //! \brief Creates a new %pipe.
    //!
    //! The default pipe constructor allocates a new anonymous %pipe
    //! and assigns its ownership to the created pipe object.
    //!
    //! \throw system_error If the anonymous %pipe creation fails.
    //!
    pipe(void);

    //!
    //! \brief Returns the %pipe's read end file handle.
    //!
    //! Obtains a reference to the %pipe's read end file handle.  Care
    //! should be taken to not duplicate the returned object if ownership
    //! shall remain to the %pipe.
    //!
    //! Duplicating the returned object invalidates its corresponding file
    //! handle in the %pipe.
    //!
    //! \return A reference to the %pipe's read end file handle.
    //!
    file_handle& rend(void);

    //!
    //! \brief Returns the %pipe's write end file handle.
    //!
    //! Obtains a reference to the %pipe's write end file handle.  Care
    //! should be taken to not duplicate the returned object if ownership
    //! shall remain to the %pipe.
    //!
    //! Duplicating the returned object invalidates its corresponding file
    //! handle in the %pipe.
    //!
    //! \return A reference to the %pipe's write end file handle.
    //!
    file_handle& wend(void);
};

} // namespace atf

#endif // !defined(_ATF_PIPE_HPP_)
