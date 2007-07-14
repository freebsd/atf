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
//! \file atfprivate/postream.hpp
//!
//! Includes the declaration of the postream class.
//!

#if !defined(_ATF_POSTREAM_HPP_)
#define _ATF_POSTREAM_HPP_

#include <ostream>

#include "atfprivate/file_handle.hpp"
#include "atfprivate/systembuf.hpp"

namespace atf {

//!
//! \brief Child process' input stream.
//!
//! The postream class represents an input communication channel with the
//! child process.  The child process reads data from this stream and the
//! parent process can write to it through the postream object.  In other
//! words, from the child's point of view, the communication channel is an
//! input one, but from the parent's point of view it is an output one;
//! hence the confusing postream name.
//!
//! postream objects cannot be copied because they own the file handle
//! they use to communicate with the child and because they buffer data
//! that flows through the communication channel.
//!
//! A postream object behaves as a std::ostream stream in all senses.
//! The class is only provided because it must provide a method to let
//! the caller explicitly close the communication channel.
//!
//! \remark <b>Blocking remarks</b>: Functions that write data to this
//! stream can block if the associated file handle blocks during the write.
//! As this class is used to communicate with child processes through
//! anonymous pipes, the most typical blocking condition happens when the
//! child is not processing the data in the pipe's system buffer.  When
//! this happens, the buffer eventually fills up and the system blocks
//! until the reader consumes some data, leaving some new room.
//!
class postream :
    public std::ostream // boost::noncopyable
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
    //! \brief Creates a new process' input stream.
    //!
    //! Given a file handle, this constructor creates a new postream
    //! object that owns the given file handle \a fh.  Ownership of
    //! \a fh is transferred to the created postream object.
    //!
    //! \pre \a fh is valid.
    //! \post \a fh is invalid.
    //! \post The new postream object owns \a fh.
    //!
    explicit postream(file_handle& fh);

    //!
    //! \brief Closes the file handle managed by this stream.
    //!
    //! Explicitly closes the file handle managed by this stream.  This
    //! function can be used by the user to tell the child process there
    //! is no more data to send.
    //!
    void close(void);
};

} // namespace atf

#endif // !defined(_ATF_POSTREAM_HPP_)
