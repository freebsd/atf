/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/wait.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <atf-c/check.h>
#include <atf-c/config.h>
#include <atf-c/dynstr.h>
#include <atf-c/fs.h>
#include <atf-c/sanity.h>

/* ---------------------------------------------------------------------
 * The "atf_check_result" type.
 * --------------------------------------------------------------------- */

atf_error_t
atf_check_exec(atf_check_result_t *r, char *const *argv)
{
    int fd_out, fd_err;
    int status;
    pid_t pid;
    atf_error_t err;
    const char *workdir;

    atf_object_init(&r->m_object);

    workdir = atf_config_get("atf_workdir");
    err = atf_fs_path_init_fmt(&r->m_stdout, "%s/%s",
                               workdir, "stdout.XXXXXX");
    if (atf_is_error(err))
        goto out_mstdout;

    err = atf_fs_path_init_fmt(&r->m_stderr, "%s/%s",
                               workdir, "stderr.XXXXXX");
    if (atf_is_error(err))
        goto out_mstderr;


    err = atf_fs_mkstemp(&r->m_stdout, &fd_out);
    if (atf_is_error(err))
        goto out_fdout;

    err = atf_fs_mkstemp(&r->m_stderr, &fd_err);
    if (atf_is_error(err))
        goto out_fderr;


    pid = fork();
    if (pid < 0) {
        err = atf_libc_error(errno, "Failed to fork");
        goto out_fork;
    } else if (pid == 0) {
        dup2(fd_out, STDOUT_FILENO);
        dup2(fd_err, STDERR_FILENO);
        if (execv(argv[0], argv) == -1)
            exit(255);
    };

    close(fd_out);
    close(fd_err);

    if (waitpid(pid, &status, 0) == -1) {
        err = atf_libc_error(errno, "Error waiting for "
                             "child process: %d", pid);
        goto out_waitpid;
    }

    if (!WIFEXITED(status)) {
        err = atf_libc_error(errno, "Error while executing "
                             "command: '%s'", argv[0]);
        goto out_wifexited;
    }

    r->m_status = WEXITSTATUS(status);

    return err;

out_wifexited:
out_waitpid:
out_fork:
    close(fd_err);
    atf_fs_unlink(&r->m_stderr);

out_fderr:
    close(fd_out);
    atf_fs_unlink(&r->m_stdout);

out_fdout:
out_mstderr:
    atf_fs_path_fini(&r->m_stderr);

out_mstdout:
    atf_fs_path_fini(&r->m_stdout);

    atf_object_fini(&r->m_object);

    return err;
}

void
atf_check_result_fini(atf_check_result_t *r)
{
    atf_fs_unlink(&r->m_stdout);
    atf_fs_path_fini(&r->m_stdout);

    atf_fs_unlink(&r->m_stderr);
    atf_fs_path_fini(&r->m_stderr);

    atf_object_fini(&r->m_object);
}

const atf_fs_path_t *
atf_check_result_stdout(const atf_check_result_t *r)
{
    return &r->m_stdout;
}

const atf_fs_path_t *
atf_check_result_stderr(const atf_check_result_t *r)
{
    return &r->m_stderr;
}

int
atf_check_result_status(const atf_check_result_t *r)
{
    return r->m_status;
}
