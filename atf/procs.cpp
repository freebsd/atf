//
// Automated Testing Framework (atf)
//
// Copyright (c) 2008 The NetBSD Foundation, Inc.
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

#if defined(HAVE_CONFIG_H)
#   include "config.h"
#endif // defined(HAVE_CONFIG_H)

#if defined(HAVE_KVM_GETPROCS)
#   define PID_GRABBER_KVM_GETPROCS
#elif defined(HAVE_SYSCTL_KERN_PROC)
#   define PID_GRABBER_SYSCTL_KERN_PROC
#elif defined(HAVE_LINUX_PROCFS)
#   define PID_GRABBER_LINUX_PROCFS
#else
#   define PID_GRABBER_OTHER
#endif

extern "C" {
#include <signal.h>
#include <unistd.h>
#if defined(PID_GRABBER_KVM_GETPROCS)
#   include <fcntl.h>
#   include <kvm.h>
#   include <sys/sysctl.h>
#endif // defined(PID_GRABBER_KVM_GETPROCS)
#if defined(PID_GRABBER_SYSCTL_KERN_PROC)
#   include <sys/types.h>
#   include <sys/sysctl.h>
#endif // defined(PID_GRABBER_SYSCTL_KERN_PROC)
#if defined(PID_GRABBER_LINUX_PROCFS)
#   include <sys/types.h>
#   include <dirent.h>
#endif // defined(PID_GRABBER_LINUX_PROCFS)
}

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "atf/exceptions.hpp"
#include "atf/procs.hpp"
#include "atf/sanity.hpp"
#include "atf/text.hpp"
#include "atf/utils.hpp"

namespace impl = atf::procs;
#define IMPL_NAME "atf::procs"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

static
bool
kill(pid_t pid, int signo, impl::errors_vector& errors)
{
    int ret;

    ret = ::kill(pid, signo);
    if (ret == -1) {
        const std::string pidstr = atf::text::to_string(pid);
        const std::string signostr = atf::text::to_string(signo);
        atf::system_error e(IMPL_NAME "::kill",
                            "kill(" + pidstr + ", " +
                            signostr + ") failed",
                            errno);
        errors.push_back(impl::pid_error_pair(pid, e.what()));
    }
    return ret != -1;
}

// ------------------------------------------------------------------------
// The "pid_grabber" class for systems with KVM.
// ------------------------------------------------------------------------

#if defined(PID_GRABBER_KVM_GETPROCS)

impl::pid_grabber::pid_grabber(void)
{
    m_cookie = ::kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
    if (m_cookie == NULL)
        throw std::runtime_error("Failed to initialize the KVM library");
}

impl::pid_grabber::~pid_grabber(void)
{
    ::kvm_close(reinterpret_cast< kvm_t * >(m_cookie));
}

bool
impl::pid_grabber::can_get_children_of(void)
    const
{
    return true;
}

impl::pid_set
impl::pid_grabber::get_children_of(pid_t pid)
{
    int cnt;
    struct kinfo_proc *procs =
        ::kvm_getprocs(reinterpret_cast< kvm_t * >(m_cookie),
                       KERN_PROC_ALL, 0, &cnt);

    pid_set children;
    for (int i = 0; i < cnt; i++) {
        if (procs[i].kp_eproc.e_ppid == pid)
            children.insert(procs[i].kp_proc.p_pid);
    }
    return children;
}

#endif // defined(PID_GRABBER_KVM_GETPROCS)

// ------------------------------------------------------------------------
// The "pid_grabber" class for systems with kern.proc sysctl nodes.
// ------------------------------------------------------------------------

#if defined(PID_GRABBER_SYSCTL_KERN_PROC)

impl::pid_grabber::pid_grabber(void) :
    m_cookie(NULL)
{
}

impl::pid_grabber::~pid_grabber(void)
{
}

bool
impl::pid_grabber::can_get_children_of(void)
    const
{
    return true;
}

impl::pid_set
impl::pid_grabber::get_children_of(pid_t pid)
{
    static int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };

    atf::utils::auto_array< struct kinfo_proc > result(NULL);
    size_t nprocs = 0;
    do {
        int err;
        size_t length;

        INV(result.get() == NULL);

        length = 0;
        err = ::sysctl(static_cast< int * >(name),
                       (sizeof(name) / sizeof(*name)) - 1,
                       NULL, &length, NULL, 0);
        if (err == -1)
            throw atf::system_error("get_children_of", "sysctl failed",
                                    errno);
        nprocs = length / sizeof(struct kinfo_proc);

        result.reset(new struct kinfo_proc[nprocs]);

        err = ::sysctl(static_cast< int * >(name),
                       (sizeof(name) / sizeof(*name)) - 1,
                       result.get(), &length, NULL, 0);
        if (err == -1) {
            if (errno == ENOMEM)
                result.reset(NULL);
            else
                throw atf::system_error("get_children_of", "sysctl failed",
                                        errno);
        }
    } while (result.get() == NULL);

    pid_set children;
    for (int i = 0; i < nprocs; i++)
        if (result[i].kp_eproc.e_ppid == pid)
            children.insert(result[i].kp_proc.p_pid);
    return children;
}

#endif // defined(PID_GRABBER_SYSCTL_KERN_PROC)

// ------------------------------------------------------------------------
// The "pid_grabber" class for systems with the Linux proc file system.
// ------------------------------------------------------------------------

#if defined(PID_GRABBER_LINUX_PROCFS)

impl::pid_grabber::pid_grabber(void) :
    m_cookie(NULL)
{
}

impl::pid_grabber::~pid_grabber(void)
{
}

bool
impl::pid_grabber::can_get_children_of(void)
    const
{
    return true;
}

static
pid_t
get_parent_pid_of(const std::string& pidstr)
{
    std::ifstream is(("/proc/" + pidstr + "/stat").c_str());
    if (!is)
        throw std::runtime_error("Cannot determine parent pid of " +
                                 pidstr);

    std::string pid;
    std::string name;
    std::string status;
    pid_t ppid;
    is >> pid >> name >> status >> ppid;

    return ppid;
}

impl::pid_set
impl::pid_grabber::get_children_of(pid_t pid)
{
    pid_set children;

    DIR* dir = ::opendir("/proc");
    try {
        struct dirent* de;
        while ((de = readdir(dir)) != NULL) {
            if (de->d_type == DT_DIR && std::isdigit(de->d_name[0])) {
                if (get_parent_pid_of(de->d_name) == pid)
                    children.insert
                        (atf::text::to_type< pid_t >(de->d_name));
            }
        }
    } catch (...) {
        ::closedir(dir);
        throw;
    }
    ::closedir(dir);

    return children;
}

#endif // defined(PID_GRABBER_LINUX_PROCFS)

// ------------------------------------------------------------------------
// The "pid_grabber" class for unsupported systems.
// ------------------------------------------------------------------------

#if defined(PID_GRABBER_OTHER)

impl::pid_grabber::pid_grabber(void) :
    m_cookie(NULL)
{
}

impl::pid_grabber::~pid_grabber(void)
{
}

bool
impl::pid_grabber::can_get_children_of(void)
    const
{
    return false;
}

impl::pid_set
impl::pid_grabber::get_children_of(pid_t pid)
{
    UNREACHABLE;
    return pid_set();
}

#endif // defined(PID_GRABBER_OTHER)

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

impl::errors_vector
impl::kill_tree(pid_t pid, int signo, pid_grabber& pg)
{
    errors_vector errors;
    if (!pg.can_get_children_of()) {
        errors.push_back
            (pid_error_pair(pid,
                            "Only killing this process because this "
                            "platform is currently unsupported"));
        (void)kill(pid, signo, errors);
    } else {
        if (kill(pid, SIGSTOP, errors) == -1)
            errors.push_back
                (pid_error_pair(pid, "Some children may not be killed"));

        pid_set children = pg.get_children_of(pid);

        for (pid_set::const_iterator iter = children.begin();
             iter != children.end(); iter++) {
            errors_vector aux = kill_tree(*iter, signo, pg);
            errors.insert(errors.end(), aux.begin(), aux.end());
        }

        if (signo == SIGKILL)
            (void)kill(pid, signo, errors);
        else {
#if defined(SUPPORT_SIGNAL_WHILE_STOPPED)
            (void)kill(pid, signo, errors);
            (void)kill(pid, SIGCONT, errors);
#else // !defined(SUPPORT_SIGNAL_WHILE_STOPPED)
            (void)kill(pid, SIGCONT, errors);
            (void)kill(pid, signo, errors);
#endif // defined(SUPPORT_SIGNAL_WHILE_STOPPED)
        }
    }
    return errors;
}
