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

//!
//! \file atfprivate/pistream.hpp
//!
//! Includes the declaration of the pistream class.
//!

#ifndef _ATF_PISTREAM_HPP_
#define _ATF_PISTREAM_HPP_

#include <istream>

#include "atfprivate/file_handle.hpp"
#include "atfprivate/systembuf.hpp"

namespace atf {

//!
//! \brief Child process' output stream.
//!
//! The pistream class represents an output communication channel with the
//! child process.  The child process writes data to this stream and the
//! parent process can read it through the pistream object.  In other
//! words, from the child's point of view, the communication channel is an
//! output one, but from the parent's point of view it is an input one;
//! hence the confusing pistream name.
//!
//! pistream objects cannot be copied because they own the file handle
//! they use to communicate with the child and because they buffer data
//! that flows through the communication channel.
//!
//! A pistream object behaves as a std::istream stream in all senses.
//! The class is only provided because it must provide a method to let
//! the caller explicitly close the communication channel.
//!
//! \remark <b>Blocking remarks</b>: Functions that read data from this
//! stream can block if the associated file handle blocks during the read.
//! As this class is used to communicate with child processes through
//! anonymous pipes, the most typical blocking condition happens when the
//! child has no more data to send to the pipe's system buffer.  When
//! this happens, the buffer eventually empties and the system blocks
//! until the writer generates some data.
//!
class pistream :
    public std::istream // XXX boost::noncopyable
{
    //!
    //! \brief The file handle managed by this stream.
    //!
    file_handle m_handle;

    //!
    //! \brief The systembuf object used to manage this stream's data.
    //!
    systembuf m_systembuf;

public:
    //!
    //! \brief Creates a new process' output stream.
    //!
    //! Given a file handle, this constructor creates a new pistream
    //! object that owns the given file handle \a fh.  Ownership of
    //! \a fh is transferred to the created pistream object.
    //!
    //! \pre \a fh is valid.
    //! \post \a fh is invalid.
    //! \post The new pistream object owns \a fh.
    //!
    explicit pistream(file_handle& fh);

    //!
    //! \brief Closes the file handle managed by this stream.
    //!
    //! Explicitly closes the file handle managed by this stream.  This
    //! function can be used by the user to tell the child process it's
    //! not willing to receive more data.
    //!
    void close(void);
};

} // namespace atf

#endif // _ATF_PISTREAM_HPP_
