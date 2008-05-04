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


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <atf-c/check.h>
#include <atf-c/config.h>
#include <atf-c/dynstr.h>
#include <atf-c/fs.h>
#include <atf-c/sanity.h>


/* ---------------------------------------------------------------------
 * The "atf_cmd_result" type.
 * --------------------------------------------------------------------- */

atf_error_t
atf_cmd_run(atf_cmd_result_t *r, const char *command)
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
        goto out_1;

    err = atf_fs_path_init_fmt(&r->m_stderr, "%s/%s",
                               workdir, "stderr.XXXXXX");
    if (atf_is_error(err))
        goto out_2;


    err = atf_fs_mkstemp(&r->m_stdout, &fd_out);
    if (atf_is_error(err))
        goto out_2;

    err = atf_fs_mkstemp(&r->m_stderr, &fd_err);
    if (atf_is_error(err))
        goto out_3;


    pid = fork();
    if (pid < 0) {
        err = atf_libc_error(errno, "Failed to fork");
        goto out_4;
    } else if (pid == 0) {
        dup2(fd_out, STDOUT_FILENO);
        dup2(fd_err, STDERR_FILENO);
        if (execl("/bin/sh", "/bin/sh", "-c", command, NULL) == -1)
            exit(254);
    };
   
    close(fd_out);
    close(fd_err);

    if (waitpid(pid, &status, 0) == -1) {
        err = atf_libc_error(errno, "Error waiting for "
                             "child process: %d", pid);
        goto out_4;
    }

    if (!WIFEXITED(status)) {
        err = atf_libc_error(errno, "Error while executing "
                             "command: '%s'", command);
        goto out_4;
    }

    r->m_status = WEXITSTATUS(status);

    return err;

out_4:
    close(fd_err);
    atf_fs_unlink(&r->m_stderr);

out_3:
    close(fd_out);
    atf_fs_unlink(&r->m_stdout);

out_2:
    atf_fs_path_fini(&r->m_stderr);

out_1:
    atf_fs_path_fini(&r->m_stdout);

    atf_object_fini(&r->m_object);

    return err;
}

void
atf_cmd_result_fini(atf_cmd_result_t *r) {
    atf_fs_unlink(&r->m_stdout);
    atf_fs_path_fini(&r->m_stdout);

    atf_fs_unlink(&r->m_stderr);
    atf_fs_path_fini(&r->m_stderr);

    atf_object_fini(&r->m_object);
}

const atf_fs_path_t *
atf_cmd_result_stdout(const atf_cmd_result_t *r)
{
    return &r->m_stdout;
}

const atf_fs_path_t *
atf_cmd_result_stderr(const atf_cmd_result_t *r)
{
    return &r->m_stderr;
}

int
atf_cmd_result_status(const atf_cmd_result_t *r)
{
    return r->m_status;
}


/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

bool
atf_equal_file_file(atf_error_t *err, const char *path1, const char *path2)
{
    bool ret;
    int fd1, fd2;
    char buf1[512], buf2[512];

    ret = false;
    *err = atf_no_error();

    fd1 = open(path1, O_RDONLY);
    if (fd1 == -1) {
        *err = atf_libc_error(errno, "Cannot open file: %s", path1);
        goto out_fd1;
    }

    fd2 = open(path2, O_RDONLY);
    if (fd2 == -1) {
        *err = atf_libc_error(errno, "Cannot open file: %s", path2);
        goto out_fd2;
    }

    while (1) {
        ssize_t r1, r2;

        r1 = read(fd1, buf1, sizeof(buf1));
        if (r1 < 0) {
            *err = atf_libc_error(errno, "Cannot read file: %s", path1);
            break;
        }

        r2 = read(fd2, buf2, sizeof(buf2));
        if (r2 < 0) {
            *err = atf_libc_error(errno, "Cannot read file: %s", path2);
            break;
        }

        if ((r1 == 0) && (r2 == 0)) {
            ret = true;
            break;
        }

        if ((r1 != r2) || (memcmp(buf1, buf2, r1) != 0)) {
            ret = false;
            break;
        }
    }


    close(fd2);

out_fd2:
    close(fd1);

out_fd1:
    return ret;
}


bool
atf_equal_file_string(atf_error_t *err, const char *path, const char *str)
{
    bool ret;
    int fd;
    size_t len;
    ssize_t cnt;
    char *buf;

    ret = false;
    *err = atf_no_error();

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        *err = atf_libc_error(errno, "Cannot open file: %s", path);
        goto out_fd;
    }

    len = strlen(str);
    buf = malloc(sizeof(char) * (len + 2));
    if (buf == NULL) {
        *err = atf_no_memory_error();
        goto out_malloc;
    }
    buf[len+1] = 0;

    cnt = read(fd, buf, len + 2);
    if (cnt < 0) {
        *err = atf_libc_error(errno, "Cannot read file: %s", path);
        goto out_read;
    }
    
    ret = ((strncmp(str, buf, len) == 0) &&
           (buf[len] == '\n') &&
           (buf[len+1] == 0));

out_read:
    free(buf);

out_malloc:
    close(fd);

out_fd:
    return ret;
}
