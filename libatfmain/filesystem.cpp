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

extern "C" {
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
}

#include <cerrno>

#include "libatfmain/exceptions.hpp"
#include "libatfmain/filesystem.hpp"

namespace am = atf::main;

am::directory::directory(const std::string& path)
{
    DIR* dp = ::opendir(path.c_str());
    if (dp == NULL)
        throw system_error("atf::main::directory::directory",
                           "opendir(3) failed", errno);

    struct dirent* dep;
    while ((dep = ::readdir(dp)) != NULL)
        m_entries.insert(dep->d_name);

    if (::closedir(dp) == -1)
        throw system_error("atf::main::directory::directory",
                           "closedir(3) failed", errno);
}

bool
am::directory::has(const std::string& name)
    const
{
    return m_entries.find(name) != m_entries.end();
}

std::string
am::get_branch_path(const std::string& path)
{
    return ::dirname(path.c_str());
}

std::string
am::get_leaf_name(const std::string& path)
{
    return ::basename(path.c_str());
}

std::string
am::get_work_dir(void)
{
#if defined(MAXPATHLEN)
    char buf[MAXPATHLEN];

    if (::getcwd(buf, sizeof(buf)) == NULL)
        throw system_error("atf::main::get_work_dir",
                           "getcwd(3) failed", errno);

    return std::string(buf);
#else // !defined(MAXPATHLEN)
#   error "Not implemented."
#endif // defined(MAXPATHLEN)
}
