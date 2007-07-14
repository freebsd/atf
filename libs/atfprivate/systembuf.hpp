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

#if !defined(_ATF_SYSTEMBUF_HPP_)
#define _ATF_SYSTEMBUF_HPP_

//!
//! \file atf/systembuf.hpp
//!
//! Includes the declaration of the systembuf class.  This file is for
//! internal usage only and must not be included by the library user.
//!

#include <streambuf>

namespace atf {

// ------------------------------------------------------------------------

//!
//! \brief std::streambuf implementation for system file handles.
//!
//! systembuf provides a std::streambuf implementation for system file
//! handles.  Contrarywise to file_handle, this class does \b not take
//! ownership of the native file handle; this should be taken care of
//! somewhere else.
//!
//! This class follows the expected semantics of a std::streambuf object.
//! However, it is not copyable to avoid introducing inconsistences with
//! the on-disk file and the in-memory buffers.
//!
class systembuf :
    public std::streambuf // XXX boost::noncopyable
{
public:
    typedef int handle_type;

    //!
    //! \brief Constructs a new systembuf for the given file handle.
    //!
    //! This constructor creates a new systembuf object that reads or
    //! writes data from/to the \a h native file handle.  This handle
    //! is \b not owned by the created systembuf object; the code
    //! should take care of it externally.
    //!
    //! This class buffers input and output; the buffer size may be
    //! tuned through the \a bufsize parameter, which defaults to 8192
    //! bytes.
    //!
    //! \see pistream and postream.
    //!
    explicit systembuf(handle_type h, std::size_t bufsize = 8192);
    ~systembuf(void);

private:
    //!
    //! \brief Native file handle used by the systembuf object.
    //!
    handle_type m_handle;

    //!
    //! \brief Internal buffer size used during read and write operations.
    //!
    std::size_t m_bufsize;

    //!
    //! \brief Internal buffer used during read operations.
    //!
    char* m_read_buf;

    //!
    //! \brief Internal buffer used during write operations.
    //!
    char* m_write_buf;

protected:
    //!
    //! \brief Reads new data from the native file handle.
    //!
    //! This operation is called by input methods when there are no more
    //! data in the input buffer.  The function fills the buffer with new
    //! data, if available.
    //!
    //! \pre All input positions are exhausted (gptr() >= egptr()).
    //! \post The input buffer has new data, if available.
    //! \returns traits_type::eof() if a read error occurrs or there are
    //!          no more data to be read.  Otherwise returns
    //!          traits_type::to_int_type(*gptr()).
    //!
    virtual int_type underflow(void);

    //!
    //! \brief Makes room in the write buffer for additional data.
    //!
    //! This operation is called by output methods when there is no more
    //! space in the output buffer to hold a new element.  The function
    //! first flushes the buffer's contents to disk and then clears it to
    //! leave room for more characters.  The given \a c character is
    //! stored at the beginning of the new space.
    //!
    //! \pre All output positions are exhausted (pptr() >= epptr()).
    //! \post The output buffer has more space if no errors occurred
    //!       during the write to disk.
    //! \post *(pptr() - 1) is \a c.
    //! \returns traits_type::eof() if a write error occurrs.  Otherwise
    //!          returns traits_type::not_eof(c).
    //!
    virtual int_type overflow(int c);

    //!
    //! \brief Flushes the output buffer to disk.
    //!
    //! Synchronizes the systembuf buffers with the contents of the file
    //! associated to this object through the native file handle.  The
    //! output buffer is flushed to disk and cleared to leave new room
    //! for more data.
    //!
    //! \returns 0 on success, -1 if an error occurred.
    //!
    virtual int sync(void);
};

} // namespace atf

#endif // !defined(_ATF_SYSTEMBUF_HPP_)
