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

#include "config.h"

extern "C" {
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
}

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <memory>

#include "atfprivate/exceptions.hpp"
#include "atfprivate/fs.hpp"
#include "atfprivate/ui.hpp" // XXX For split.

namespace impl = atf::fs;
#define IMPL_NAME "atf::fs"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

//!
//! \brief Normalizes a path.
//!
//! Normalizes a path string by removing any consecutive separators.
//! The returned string should be used to construct a path object
//! immediately.
//!
static
std::string
normalize(const std::string& s)
{
    if (s.empty())
        throw impl::path_error("Path cannot be empty");

    std::vector< std::string > cs = atf::split(s, "/");
    std::string data = (s[0] == '/') ? "/" : "";
    for (std::vector< std::string >::size_type i = 0; i < cs.size(); i++) {
        data += cs[i];
        if (i != cs.size() - 1)
            data += '/';
    }

    return data;
}

// ------------------------------------------------------------------------
// The "path_error" class.
// ------------------------------------------------------------------------

impl::path_error::path_error(const std::string& w) :
    std::runtime_error(w.c_str())
{
}

// ------------------------------------------------------------------------
// The "path" class.
// ------------------------------------------------------------------------

impl::path::path(const std::string& s) :
    m_data(normalize(s))
{
}

const char*
impl::path::c_str(void)
    const
{
    return m_data.c_str();
}

const std::string&
impl::path::str(void)
    const
{
    return m_data;
}

bool
impl::path::is_absolute(void)
    const
{
    return m_data[0] == '/';
}

bool
impl::path::is_root(void)
    const
{
    return m_data == "/";
}

impl::path
impl::path::branch_path(void)
    const
{
    std::string branch;

    std::string::size_type endpos = m_data.rfind('/');
    if (endpos == std::string::npos)
        branch = ".";
    else if (endpos == 0)
        branch = "/";
    else
        branch = m_data.substr(0, endpos);

#if defined(HAVE_CONST_DIRNAME)
    assert(branch == ::dirname(m_data.c_str()));
#endif // defined(HAVE_CONST_DIRNAME)

    return path(branch);
}

std::string
impl::path::leaf_name(void)
    const
{
    std::string::size_type begpos = m_data.rfind('/');
    if (begpos == std::string::npos)
        begpos = 0;
    else
        begpos++;

    std::string leaf = m_data.substr(begpos);

#if defined(HAVE_CONST_BASENAME)
    assert(leaf == ::basename(m_data.c_str()));
#endif // defined(HAVE_CONST_BASENAME)

    return leaf;
}

bool
impl::path::operator==(const path& p)
    const
{
    return m_data == p.m_data;
}

bool
impl::path::operator!=(const path& p)
    const
{
    return m_data != p.m_data;
}

impl::path
impl::path::operator/(const std::string& p)
    const
{
    return path(m_data + "/" + normalize(p));
}

impl::path
impl::path::operator/(const path& p)
    const
{
    return path(m_data + "/" + p.m_data);
}

// ------------------------------------------------------------------------
// The "file_info" class.
// ------------------------------------------------------------------------

impl::file_info::file_info(const path& p) :
    m_path(p)
{
    struct stat sb;

    if (::stat(p.c_str(), &sb) == -1)
        throw atf::system_error(IMPL_NAME "::file_info(" + p.str() + ")",
                                "stat(2) failed", errno);

    switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:  m_type = blk_type;     break;
    case S_IFCHR:  m_type = chr_type;     break;
    case S_IFDIR:  m_type = dir_type;     break;
    case S_IFIFO:  m_type = fifo_type;    break;
    case S_IFLNK:  m_type = lnk_type;     break;
    case S_IFREG:  m_type = reg_type;     break;
    case S_IFSOCK: m_type = sock_type;    break;
    case S_IFWHT:  m_type = wht_type;     break;
    default:       m_type = unknown_type; break;
    }
}

impl::file_info::file_info(const path& dir, void* data) :
    m_path("not-yet-initialized")
{
    struct dirent* de = static_cast< struct dirent* >(data);
    m_path = dir / de->d_name;
    switch (de->d_type) {
    case DT_BLK:     m_type = blk_type;     break;
    case DT_CHR:     m_type = chr_type;     break;
    case DT_DIR:     m_type = dir_type;     break;
    case DT_FIFO:    m_type = fifo_type;    break;
    case DT_LNK:     m_type = lnk_type;     break;
    case DT_REG:     m_type = reg_type;     break;
    case DT_SOCK:    m_type = sock_type;    break;
    case DT_UNKNOWN: m_type = unknown_type; break;
    case DT_WHT:     m_type = wht_type;     break;
    default:         m_type = unknown_type; break;
    }
}

const impl::path&
impl::file_info::get_path(void)
    const
{
    return m_path;
}

impl::file_info::type
impl::file_info::get_type(void)
    const
{
    return m_type;
}

// ------------------------------------------------------------------------
// The "directory" class.
// ------------------------------------------------------------------------

impl::directory::directory(const path& p)
{
    DIR* dp = ::opendir(p.c_str());
    if (dp == NULL)
        throw system_error(IMPL_NAME "::directory::directory(" +
                           p.str() + ")", "opendir(3) failed", errno);

    struct dirent* dep;
    while ((dep = ::readdir(dp)) != NULL)
        insert(value_type(dep->d_name, file_info(p, dep)));

    if (::closedir(dp) == -1)
        throw system_error(IMPL_NAME "::directory::directory(" +
                           p.str() + ")", "closedir(3) failed", errno);
}

std::set< std::string >
impl::directory::names(void)
    const
{
    std::set< std::string > ns;

    for (const_iterator iter = begin(); iter != end(); iter++)
        ns.insert((*iter).first);

    return ns;
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

impl::path
impl::change_directory(const path& dir)
{
    path olddir = get_current_dir();

    if (olddir != dir) {
        if (::chdir(dir.c_str()) == -1)
            throw system_error(IMPL_NAME "::chdir(" + dir.str() + ")",
                               "chdir(2) failed", errno);
    }

    return olddir;
}

impl::path
impl::create_temp_dir(const path& tmpl)
{
    std::auto_ptr< char > buf(new char[tmpl.str().length() + 1]);
    std::strcpy(buf.get(), tmpl.c_str());
    if (::mkdtemp(buf.get()) == NULL)
        throw system_error(IMPL_NAME "::create_temp_dir(" +
                           tmpl.str() + "/", "mkdtemp(3) failed",
                           errno);
    return path(buf.get());
}

bool
impl::exists(const path& p)
{
    bool ok;

    int res = ::access(p.c_str(), F_OK);
    if (res == 0)
        ok = true;
    else if (res == -1 && errno == ENOENT)
        ok = false;
    else
        throw system_error(IMPL_NAME "::exists(" + p.str() + ")",
                           "access(2) failed", errno);

    return ok;
}

impl::path
impl::get_current_dir(void)
{
#if defined(MAXPATHLEN)
    char buf[MAXPATHLEN];

    if (::getcwd(buf, sizeof(buf)) == NULL)
        throw system_error(IMPL_NAME "::get_current_dir",
                           "getcwd(3) failed", errno);

    return path(buf);
#else // !defined(MAXPATHLEN)
#   error "Not implemented."
#endif // defined(MAXPATHLEN)
}

void
impl::rm_rf(const path& p)
{
    assert(p.is_absolute());
    assert(!p.is_root());

    directory dir(p);
    dir.erase(".");
    dir.erase("..");

    for (directory::iterator iter = dir.begin(); iter != dir.end();
         iter++) {
        const file_info& entry = (*iter).second;

        path entryp = entry.get_path();
        if (entry.get_type() == file_info::dir_type)
            rm_rf(entryp);
        else {
            if (::unlink(entryp.c_str()) == -1)
                throw system_error(IMPL_NAME "::rm_rf(" + p.str() + ")",
                                   "unlink(" + entryp.str() + ") failed",
                                   errno);
        }
    }

    if (::rmdir(p.c_str()) == -1)
        throw system_error(IMPL_NAME "::rm_rf(" + p.str() + ")",
                           "rmdir(" + p.str() + ") failed", errno);
}
