//
// Automated Testing Framework (atf)
//
// Copyright (c) 2008, 2009 The NetBSD Foundation, Inc.
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

#if !defined(_ATF_CXX_PROCESS_HPP_)
#define _ATF_CXX_PROCESS_HPP_

extern "C" {
#include <sys/types.h>

#include <atf-c/error.h>
#include <atf-c/process.h>
}

#include <string>

#include <atf-c++/exceptions.hpp>
#include <atf-c++/fs.hpp>
#include <atf-c++/io.hpp>

namespace atf {
namespace process {

class child;
class status;

// ------------------------------------------------------------------------
// The "stream" types.
// ------------------------------------------------------------------------

class basic_stream {
protected:
    atf_process_stream_t m_sb;
    bool m_inited;

    const atf_process_stream_t* get_sb(void) const;

public:
    basic_stream(void);
    ~basic_stream(void);
};

class stream_capture : basic_stream {
    // Allow access to the getters.
    template< class OutStream, class ErrStream > friend
    child fork(void (*)(const void*), const OutStream&, const ErrStream&,
               const void*);

public:
    stream_capture(void);
};

class stream_inherit : basic_stream {
    // Allow access to the getters.
    template< class OutStream, class ErrStream > friend
    child fork(void (*)(const void*), const OutStream&, const ErrStream&,
               const void*);

public:
    stream_inherit(void);
};

class stream_redirect_fd : basic_stream {
    // Allow access to the getters.
    template< class OutStream, class ErrStream > friend
    child fork(void (*)(const void*), const OutStream&, const ErrStream&,
               const void*);

public:
    stream_redirect_fd(const int);
};

class stream_redirect_path : basic_stream {
    // Allow access to the getters.
    template< class OutStream, class ErrStream > friend
    child fork(void (*)(const void*), const OutStream&, const ErrStream&,
               const void*);

public:
    stream_redirect_path(const fs::path&);
};

// ------------------------------------------------------------------------
// The "status" type.
// ------------------------------------------------------------------------

class status {
    atf_process_status_t m_status;

    friend class child;

    status(atf_process_status_t&);

public:
    ~status(void);

    bool exited(void) const;
    int exitstatus(void) const;

    bool signaled(void) const;
    int termsig(void) const;
    bool coredump(void) const;
};

// ------------------------------------------------------------------------
// The "child" type.
// ------------------------------------------------------------------------

class child {
    atf_process_child_t m_child;

    template< class OutStream, class ErrStream > friend
    child fork(void (*)(const void*), const OutStream&, const ErrStream&,
               const void*);

    child(atf_process_child_t& c);

public:
    ~child(void);

    status wait(void);

    pid_t pid(void) const;
    io::file_handle stdout_fd(void);
    io::file_handle stderr_fd(void);
};

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

template< class OutStream, class ErrStream >
child
fork(void (*start)(const void*), const OutStream& outsb,
     const ErrStream& errsb, const void* v)
{
    atf_process_child_t c;

    atf_error_t err = atf_process_fork(&c, start, outsb.get_sb(),
                                       errsb.get_sb(), v);
    if (atf_is_error(err))
        throw_atf_error(err);

    return child(c);
}

pid_t oldfork(void);

void system(const std::string&);

} // namespace process
} // namespace atf

#endif // !defined(_ATF_CXX_PROCESS_HPP_)
