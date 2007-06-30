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

#ifndef _ATF_FILE_HANDLE_HPP_
#define _ATF_FILE_HANDLE_HPP_

//!
//! \file tools/file_handle.hpp
//!
//! Includes the declaration of the file_handle class.  This file is for
//! internal usage only and must not be included by the library user.
//!

namespace atf {

//!
//! \brief Simple RAII model for system file handles.
//!
//! The \a file_handle class is a simple RAII model for native system file
//! handles.  This class wraps one of such handles grabbing its ownership,
//! and automaticaly closes it upon destruction.  It is basically used
//! inside the library to avoid leaking open file handles, shall an
//! unexpected execution trace occur.
//!
//! A \a file_handle object can be copied but doing so invalidates the
//! source object.  There can only be a single valid \a file_handle object
//! for a given system file handle.  This is similar to std::auto_ptr\<\>'s
//! semantics.
//!
//! This class also provides some convenience methods to issue special file
//! operations under their respective platforms.
//!
class file_handle
{
public:
    //!
    //! \brief Opaque name for the native handle type.
    //!
    //! Each operating system identifies file handles using a specific type.
    //! The \a handle_type type is used to transparently refer to file
    //! handles regarless of the operating system in which this class is
    //! used.
    //!
    //! If this class is used in a POSIX system, \a NativeSystemHandle is
    //! an integer type while it is a \a HANDLE in a Win32 system.
    //!
    typedef int handle_type;

    //!
    //! \brief Constructs an invalid file handle.
    //!
    //! This constructor creates a new \a file_handle object that represents
    //! an invalid file handle.  An invalid file handle can be copied but
    //! cannot be manipulated in any way (except checking for its validity).
    //!
    //! \see is_valid()
    //!
    file_handle(void);

    //!
    //! \brief Constructs a new file handle from a native file handle.
    //!
    //! This constructor creates a new \a file_handle object that takes
    //! ownership of the given \a h native file handle.  The user must not
    //! close \a h on his own during the lifetime of the new object.
    //! Ownership can be reclaimed using disown().
    //!
    //! \pre The native file handle must be valid; a close operation must
    //!      succeed on it.
    //!
    //! \see disown()
    //!
    file_handle(handle_type h);

    //!
    //! \brief Copy constructor; invalidates the source handle.
    //!
    //! This copy constructor creates a new file handle from a given one.
    //! Ownership of the native file handle is transferred to the new
    //! object, effectively invalidating the source file handle.  This
    //! avoids having two live \a file_handle objects referring to the
    //! same native file handle.  The source file handle need not be
    //! valid in the name of simplicity.
    //!
    //! \post The source file handle is invalid.
    //! \post The new file handle owns the source's native file handle.
    //!
    file_handle(const file_handle& fh);

    //!
    //! \brief Releases resources if the handle is valid.
    //!
    //! If the file handle is valid, the destructor closes it.
    //!
    //! \see is_valid()
    //!
    ~file_handle(void);

    //!
    //! \brief Assignment operator; invalidates the source handle.
    //!
    //! This assignment operator transfers ownership of the RHS file
    //! handle to the LHS one, effectively invalidating the source file
    //! handle.  This avoids having two live \a file_handle objects
    //! referring to the same native file handle.  The source file
    //! handle need not be valid in the name of simplicity.
    //!
    //! \post The RHS file handle is invalid.
    //! \post The LHS file handle owns RHS' native file handle.
    //! \return A reference to the LHS file handle.
    //!
    file_handle& operator=(const file_handle& fh);

    //!
    //! \brief Checks whether the file handle is valid or not.
    //!
    //! Returns a boolean indicating whether the file handle is valid or
    //! not.  If the file handle is invalid, no other applications can be
    //! executed other than the destructor.
    //!
    //! \return True if the file handle is valid; false otherwise.
    //!
    bool is_valid(void) const;

    //!
    //! \brief Closes the file handle.
    //!
    //! Explicitly closes the file handle, which must be valid.  Upon
    //! exit, the handle is not valid any more.
    //!
    //! \pre The file handle is valid.
    //! \post The file handle is invalid.
    //! \post The native file handle is closed.
    //!
    void close(void);

    //!
    //! \brief Reclaims ownership of the native file handle.
    //!
    //! Explicitly reclaims ownership of the native file handle contained
    //! in the \a file_handle object, returning the native file handle.
    //! The caller is responsible of closing it later on.
    //!
    //! \pre The file handle is valid.
    //! \post The file handle is invalid.
    //! \return The native file handle.
    //!
    handle_type disown(void);

    //!
    //! \brief Gets the native file handle.
    //!
    //! Returns the native file handle for the \a file_handle object.
    //! The caller can issue any operation on it except closing it.
    //! If closing is required, disown() shall be used.
    //!
    //! \pre The file handle is valid.
    //! \return The native file handle.
    //!
    handle_type get(void) const;

    //!
    //! \brief Changes the native file handle to the given one.
    //!
    //! Given a new native file handle \a h, this operation assigns this
    //! handle to the current object, closing its old native file handle.
    //! In other words, it first calls dup2() to remap the old handle to
    //! the new one and then closes the old handle.
    //!
    //! If \a h is open, it is automatically closed by dup2().
    //!
    //! This operation is only available in POSIX systems.
    //!
    //! \pre The file handle is valid.
    //! \pre The native file handle \a h is valid; i.e., it must be
    //!      closeable.
    //! \post The file handle's native file handle is \a h.
    //! \throw system_error If the internal remapping operation fails.
    //!
    void posix_remap(handle_type h);

    //!
    //! \brief Duplicates an open native file handle.
    //!
    //! Given a native file handle \a h1, this routine duplicates it so
    //! that it ends up being identified by the native file handle \a h2
    //! and returns a new \a file_handle owning \a h2.
    //!
    //! This operation is only available in POSIX systems.
    //!
    //! \pre The native file handle \a h1 is open.
    //! \pre The native file handle \a h2 is valid (non-negative).
    //! \post The native file handle \a h1 is closed.
    //! \post The native file handle \a h2 is the same as the old \a h1
    //!       from the operating system's point of view.
    //! \return A new \a file_handle object that owns \a h2.
    //! \throw system_error If dup2() fails.
    //!
    static file_handle posix_dup(int h1, int h2);

private:
    //!
    //! \brief Internal handle value.
    //!
    //! This variable holds the native handle value for the file handle
    //! hold by this object.  It is interesting to note that this needs
    //! to be mutable because the copy constructor and the assignment
    //! operator invalidate the source object.
    //!
    mutable handle_type m_handle;

    //!
    //! \brief Constant function representing an invalid handle value.
    //!
    //! Returns the platform-specific handle value that represents an
    //! invalid handle.  This is a constant function rather than a regular
    //! constant because, in the latter case, we cannot define it under
    //! Win32 due to the value being of a complex type.
    //!
    static const handle_type invalid_value(void);
};

} // namespace atf

#endif // _ATF_FILE_HANDLE_HPP_
