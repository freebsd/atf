/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008, 2009 The NetBSD Foundation, Inc.
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atf-c/build.h"
#include "atf-c/check.h"
#include "atf-c/config.h"
#include "atf-c/dynstr.h"
#include "atf-c/error.h"
#include "atf-c/fs.h"
#include "atf-c/list.h"
#include "atf-c/process.h"
#include "atf-c/sanity.h"

/* Only needed for testing, so not in the public header. */
atf_error_t atf_check_result_init(atf_check_result_t *, const char *const *);

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

static
atf_error_t
create_files(atf_check_result_t *r, int *fdout, int *fderr)
{
    atf_error_t err;

    err = atf_fs_mkstemp(&r->m_stdout, fdout);
    if (atf_is_error(err))
        goto out;

    err = atf_fs_mkstemp(&r->m_stderr, fderr);
    if (atf_is_error(err))
        goto err_fdout;

    INV(!atf_is_error(err));
    goto out;

err_fdout:
    close(*fdout);
    atf_fs_unlink(&r->m_stdout);
out:
    return err;
}

static
void
cleanup_files(const atf_check_result_t *r, int fdout, int fderr)
{
    int ret;

    ret = close(fdout);
    INV(ret == 0);
    ret = close(fderr);
    INV(ret == 0);

    atf_fs_unlink(&r->m_stdout);
    atf_fs_unlink(&r->m_stderr);
}

static
int
const_execvp(const char *file, const char *const *argv)
{
#define UNCONST(a) ((void *)(unsigned long)(const void *)(a))
    return execvp(file, UNCONST(argv));
#undef UNCONST
}

static
atf_error_t
fork_and_wait(const char *const *argv, int fdout, int fderr, int *estatus)
{
    atf_error_t err;
    int status;
    pid_t pid;

    err = atf_process_fork(&pid);
    if (atf_is_error(err))
        goto out;

    if (pid == 0) {
        atf_disable_exit_checks();
        /* XXX No error handling at all? */
        dup2(fdout, STDOUT_FILENO);
        dup2(fderr, STDERR_FILENO);
        const_execvp(argv[0], argv);
        fprintf(stderr, "execvp(%s) failed: %s", argv[0], strerror(errno));
        exit(127);
        UNREACHABLE;
    } else {
        if (waitpid(pid, &status, 0) == -1) {
            err = atf_libc_error(errno, "Error waiting for "
                                 "child process: %d", pid);
        } else {
            *estatus = status;
        }
    }

out:
    return err;
}

static
atf_error_t
array_to_list(const char *const *a, atf_list_t *l)
{
    atf_error_t err;

    err = atf_list_init(l);
    if (atf_is_error(err))
        goto out;

    while (*a != NULL) {
        char *item = strdup(*a);
        if (item == NULL) {
            err = atf_no_memory_error();
            goto out;
        }

        err = atf_list_append(l, item, true);
        if (atf_is_error(err))
            goto out;

        a++;
    }

out:
    return err;
}

static
atf_error_t
list_to_array(const atf_list_t *l, const char ***ap)
{
    atf_error_t err;
    const char **a;

    a = (const char **)malloc(atf_list_size(l) * sizeof(const char *));
    if (a == NULL)
        err = atf_no_memory_error();
    else {
        const char **aiter;
        atf_list_citer_t liter;

        aiter = a;
        atf_list_for_each_c(liter, l) {
            *aiter = (const char *)atf_list_citer_data(liter);
            aiter++;
        }

        err = atf_no_error();
        *ap = a;
    }

    return err;
}

static
void
print_list(const atf_list_t *l, const char *pfx)
{
    atf_list_citer_t iter;

    printf("%s", pfx);
    atf_list_for_each_c(iter, l)
        printf(" %s", (const char *)atf_list_citer_data(iter));
    printf("\n");
}

/* ---------------------------------------------------------------------
 * The "atf_check_result" type.
 * --------------------------------------------------------------------- */

atf_error_t
atf_check_result_init(atf_check_result_t *r, const char *const *argv)
{
    atf_error_t err;
    const char *workdir;

    atf_object_init(&r->m_object);

    workdir = atf_config_get("atf_workdir");

    err = array_to_list(argv, &r->m_argv);
    if (atf_is_error(err))
        goto out;

    err = atf_fs_path_init_fmt(&r->m_stdout, "%s/%s",
                               workdir, "stdout.XXXXXX");
    if (atf_is_error(err))
        goto err_argv;

    err = atf_fs_path_init_fmt(&r->m_stderr, "%s/%s",
                               workdir, "stderr.XXXXXX");
    if (atf_is_error(err))
        goto err_stdout;

    INV(!atf_is_error(err));
    goto out;

err_stdout:
    atf_fs_path_fini(&r->m_stdout);
err_argv:
    atf_list_fini(&r->m_argv);
out:
    return err;
}

void
atf_check_result_fini(atf_check_result_t *r)
{
    atf_fs_unlink(&r->m_stdout);
    atf_fs_path_fini(&r->m_stdout);

    atf_fs_unlink(&r->m_stderr);
    atf_fs_path_fini(&r->m_stderr);

    atf_list_fini(&r->m_argv);

    atf_object_fini(&r->m_object);
}

const atf_list_t *
atf_check_result_argv(const atf_check_result_t *r)
{
    return &r->m_argv;
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

bool
atf_check_result_exited(const atf_check_result_t *r)
{
    int estatus = r->m_estatus;
    return WIFEXITED(estatus);
}

int
atf_check_result_exitcode(const atf_check_result_t *r)
{
    int estatus = r->m_estatus;
    return WEXITSTATUS(estatus);
}

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

/* XXX: This function shouldn't be in this module.  It messes with stdout
 * and stderr, and it provides a very high-end interface.  This belongs,
 * probably, somewhere related to test cases (such as in the tc module). */
atf_error_t
atf_check_build_c_o(const char *sfile,
                    const char *ofile,
                    const char *const optargs[],
                    bool *success)
{
    atf_error_t err;
    atf_list_t argv;
    const char **argva;
    int status;

    err = atf_build_c_o(sfile, ofile, optargs, &argv);
    if (atf_is_error(err))
        goto out;

    print_list(&argv, ">");

    err = list_to_array(&argv, &argva);
    if (atf_is_error(err))
        goto out_argv;

    err = fork_and_wait(argva, STDOUT_FILENO, STDERR_FILENO, &status);
    if (atf_is_error(err))
        goto out_argva;

    *success = WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS;

    if (!WIFEXITED(status))
        fprintf(stderr, "%s abnormally exited with status %d\n",
                argva[0], status);
    else if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
        fprintf(stderr, "%s failed with exit code %d\n",
                argva[0], WEXITSTATUS(status));
    else
        INV(*success);

    err = atf_no_error();

out_argva:
    free(argva);
out_argv:
    atf_list_fini(&argv);
out:
    return err;
}

atf_error_t
atf_check_exec_array(const char *const *argv, atf_check_result_t *r)
{
    atf_error_t err;
    int fdout, fderr;

    err = atf_check_result_init(r, argv);
    if (atf_is_error(err))
        goto out;

    err = create_files(r, &fdout, &fderr);
    if (atf_is_error(err))
        goto err_r;

    err = fork_and_wait(argv, fdout, fderr, &r->m_estatus);
    if (atf_is_error(err))
        goto err_files;

    INV(!atf_is_error(err));
    goto out;

err_files:
    cleanup_files(r, fdout, fderr);
err_r:
    atf_check_result_fini(r);
out:
    return err;
}

atf_error_t
atf_check_exec_list(const atf_list_t *argv, atf_check_result_t *r)
{
    atf_error_t err;
    const char **argv2;

    argv2 = NULL; /* Silence GCC warning. */
    err = list_to_array(argv, &argv2);
    if (atf_is_error(err))
        goto out;

    err = atf_check_exec_array(argv2, r);

    free(argv2);
out:
    return err;
}
