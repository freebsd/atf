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
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atf-c/config.h"
#include "atf-c/fs.h"
#include "atf-c/io.h"
#include "atf-c/sanity.h"
#include "atf-c/tc.h"
#include "atf-c/tcr.h"
#include "atf-c/text.h"

/* ---------------------------------------------------------------------
 * Auxiliary types and functions.
 * --------------------------------------------------------------------- */

/* Parent-only stuff. */
static atf_error_t body_parent(const atf_tc_t *, const atf_fs_path_t *,
                               pid_t, atf_tcr_t *);
static atf_error_t get_tc_result(const atf_fs_path_t *, atf_tcr_t *);
static atf_error_t parse_tc_result(int, atf_tcr_t *);

/* Child-only stuff. */
static void body_child(const atf_tc_t *, const atf_fs_path_t *)
            __attribute__((noreturn));
static void fatal_atf_error(const char *, atf_error_t)
            __attribute__((noreturn));
static void fatal_libc_error(const char *, int)
            __attribute__((noreturn));
static atf_error_t format_reason(atf_dynstr_t *, const char *, va_list);
static void write_tcr(const atf_tc_t *, const char *, const char *,
                      va_list *);

/* ---------------------------------------------------------------------
 * The "atf_tc" type.
 * --------------------------------------------------------------------- */

/*
 * Constructors/destructors.
 */

atf_error_t
atf_tc_init(atf_tc_t *tc, const char *ident, atf_tc_head_t head,
            atf_tc_body_t body, atf_tc_cleanup_t cleanup,
            const atf_map_t *config)
{
    atf_error_t err;

    atf_object_init(&tc->m_object);

    err = atf_map_init(&tc->m_vars);
    if (atf_is_error(err))
        goto err_object;

    tc->m_ident = ident;
    tc->m_head = head;
    tc->m_body = body;
    tc->m_cleanup = cleanup;
    tc->m_config = config;

    /* XXX Should the head be able to return error codes? */
    tc->m_head(tc);

    INV(!atf_is_error(err));
    return err;

err_object:
    atf_object_fini(&tc->m_object);

    return err;
}

atf_error_t
atf_tc_init_pack(atf_tc_t *tc, const atf_tc_pack_t *pack,
                 const atf_map_t *config)
{
    return atf_tc_init(tc, pack->m_ident, pack->m_head, pack->m_body,
                       pack->m_cleanup, config);
}

void
atf_tc_fini(atf_tc_t *tc)
{
    atf_map_fini(&tc->m_vars);

    atf_object_fini(&tc->m_object);
}

/*
 * Getters.
 */

const atf_map_t *
atf_tc_get_config(const atf_tc_t *tc)
{
    return tc->m_config;
}

const char *
atf_tc_get_ident(const atf_tc_t *tc)
{
    return tc->m_ident;
}

const char *
atf_tc_get_var(const atf_tc_t *tc, const char *name)
{
    const char *val;
    atf_map_citer_t iter;

    PRE(atf_tc_has_var(tc, name));
    iter = atf_map_find_c(&tc->m_vars, name);
    val = atf_map_citer_data(iter);
    INV(val != NULL);

    return val;
}

bool
atf_tc_has_var(const atf_tc_t *tc, const char *name)
{
    atf_map_citer_t end, iter;

    iter = atf_map_find_c(&tc->m_vars, name);
    end = atf_map_end_c(&tc->m_vars);
    return !atf_equal_map_citer_map_citer(iter, end);
}

/*
 * Modifiers.
 */

atf_error_t
atf_tc_set_var(atf_tc_t *tc, const char *name, const char *fmt, ...)
{
    atf_error_t err;
    char *value;
    va_list ap;

    PRE(!atf_tc_has_var(tc, name));

    va_start(ap, fmt);
    err = atf_text_format_ap(&value, fmt, ap);
    va_end(ap);

    if (!atf_is_error(err))
        err = atf_map_insert(&tc->m_vars, name, value, true);
    else
        free(value);

    return err;
}

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

atf_error_t
atf_tc_run(const atf_tc_t *tc, atf_tcr_t *tcr)
{
    atf_error_t err;
    atf_fs_path_t workdir;
    pid_t pid;

    err = atf_fs_path_init_fmt(&workdir, "%s/atf.XXXXXX",
                               atf_config_get("atf_workdir"));
    if (atf_is_error(err))
        goto out;

    err = atf_fs_mkdtemp(&workdir);
    if (atf_is_error(err))
        goto out_workdir;

    pid = fork();
    if (pid == -1) {
        err = atf_libc_error(errno, "Cannot fork to run test case body "
                             "of %s", tc->m_ident);
    } else if (pid == 0) {
        body_child(tc, &workdir);
        UNREACHABLE;
        abort();
    } else {
        err = body_parent(tc, &workdir, pid, tcr);
    }

    (void)atf_fs_cleanup(&workdir);
out_workdir:
    atf_fs_path_fini(&workdir);
out:
    return err;
}

/*
 * Parent-only stuff.
 */

static
atf_error_t
body_parent(const atf_tc_t *tc, const atf_fs_path_t *workdir, pid_t pid,
            atf_tcr_t *tcr)
{
    atf_error_t err;
    int state;

    if (waitpid(pid, &state, 0) == -1) {
        err = atf_libc_error(errno, "Error waiting for child process "
                             "%d", pid);
        goto out;
    }

    if (!WIFEXITED(state) || WEXITSTATUS(state) != EXIT_SUCCESS)
        err = atf_tcr_init_reason(tcr, atf_tcr_failed_state,
                                  "Test case did not exit cleanly; "
                                  "state was %d", state);
    else
        err = get_tc_result(workdir, tcr);

out:
    return err;
}

static
atf_error_t
get_tc_result(const atf_fs_path_t *workdir, atf_tcr_t *tcr)
{
    atf_error_t err;
    int fd;
    atf_fs_path_t tcrfile;

    err = atf_fs_path_copy(&tcrfile, workdir);
    if (atf_is_error(err))
        goto out;

    err = atf_fs_path_append_fmt(&tcrfile, "tc-result");
    if (atf_is_error(err))
        goto out_tcrfile;

    fd = open(atf_fs_path_cstring(&tcrfile), O_RDONLY);
    if (fd == -1) {
        err = atf_libc_error(errno, "Cannot retrieve test case result");
        goto out_tcrfile;
    }

    err = parse_tc_result(fd, tcr);

    close(fd);
out_tcrfile:
    atf_fs_path_fini(&tcrfile);
out:
    return err;
}

static
atf_error_t
parse_tc_result(int fd, atf_tcr_t *tcr)
{
    atf_error_t err;
    atf_dynstr_t state, reason;

    err = atf_dynstr_init(&state);
    if (atf_is_error(err))
        goto out;

    err = atf_dynstr_init(&reason);
    if (atf_is_error(err))
        goto out_state;

    err = atf_io_readline(fd, &state);
    if (atf_is_error(err))
        goto out_reason;

    if (atf_equal_dynstr_cstring(&state, "passed"))
        err = atf_tcr_init(tcr, atf_tcr_passed_state);
    else {
        err = atf_io_readline(fd, &reason);
        if (atf_is_error(err))
            goto out_reason;

        if (atf_equal_dynstr_cstring(&state, "failed"))
            err = atf_tcr_init_reason(tcr, atf_tcr_failed_state, "%s",
                                      atf_dynstr_cstring(&reason));
        else if (atf_equal_dynstr_cstring(&state, "skipped"))
            err = atf_tcr_init_reason(tcr, atf_tcr_skipped_state, "%s",
                                      atf_dynstr_cstring(&reason));
        else
            UNREACHABLE;
    }

out_reason:
    atf_dynstr_fini(&reason);
out_state:
    atf_dynstr_fini(&state);
out:
    return err;
}

/*
 * Child-only stuff.
 */

static const atf_tc_t *current_tc = NULL;
static const atf_fs_path_t *current_workdir = NULL;

static
void
body_child(const atf_tc_t *tc, const atf_fs_path_t *workdir)
{
    atf_disable_exit_checks();

    current_tc = tc;
    current_workdir = workdir;

    if (chdir(atf_fs_path_cstring(workdir)) == -1)
        atf_tc_fail("Cannot enter work directory '%s'",
                    atf_fs_path_cstring(workdir));

    tc->m_body(tc);

    atf_tc_pass();

    UNREACHABLE;
    abort();
}

static
void
fatal_atf_error(const char *prefix, atf_error_t err)
{
    char buf[1024];

    INV(atf_is_error(err));

    atf_error_format(err, buf, sizeof(buf));
    atf_error_free(err);

    fprintf(stderr, "%s: %s", prefix, buf);

    abort();
}

static
void
fatal_libc_error(const char *prefix, int err)
{
    fprintf(stderr, "%s: %s", prefix, strerror(err));

    abort();
}

static
atf_error_t
format_reason(atf_dynstr_t *reason, const char *fmt, va_list ap)
{
    atf_error_t err;
    atf_dynstr_t tmp;

    err = atf_dynstr_init_ap(&tmp, fmt, ap);
    if (atf_is_error(err))
        goto out;

    /* There is no reason for calling rfind instead of find other than
     * find is not implemented. */
    if (atf_dynstr_rfind_ch(&tmp, '\n') == atf_dynstr_npos) {
        err = atf_dynstr_copy(reason, &tmp);
    } else {
        const char *iter;

        err = atf_dynstr_init_fmt(reason, "BOGUS REASON (THE ORIGINAL "
                                  "HAD NEWLINES): ");
        if (atf_is_error(err))
            goto out_tmp;

        for (iter = atf_dynstr_cstring(&tmp); *iter != '\0'; iter++) {
            if (*iter == '\n')
                err = atf_dynstr_append_fmt(reason, "<<NEWLINE>>");
            else
                err = atf_dynstr_append_fmt(reason, "%c", *iter);

            if (atf_is_error(err)) {
                atf_dynstr_fini(reason);
                break;
            }
        }
    }

out_tmp:
    atf_dynstr_fini(&tmp);
out:
    return err;
}

static
void
write_tcr(const atf_tc_t *tc, const char *state, const char *reason,
          va_list *ap)
{
    atf_error_t err;
    int fd;
    atf_fs_path_t tcrfile;

    err = atf_fs_path_copy(&tcrfile, current_workdir);
    if (atf_is_error(err))
        fatal_atf_error("Cannot write test case results", err);

    err = atf_fs_path_append_fmt(&tcrfile, "tc-result");
    if (atf_is_error(err))
        fatal_atf_error("Cannot write test case results", err);

    fd = open(atf_fs_path_cstring(&tcrfile),
              O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (fd == -1)
        fatal_libc_error("Cannot write test case results", errno);

    err = atf_io_write_fmt(fd, "%s\n", state);
    if (atf_is_error(err))
        fatal_atf_error("Cannot write test case results", err);

    if (reason != NULL) {
        atf_dynstr_t r;

        err = format_reason(&r, reason, *ap);
        if (atf_is_error(err))
            fatal_atf_error("Cannot write test case results", err);

        INV(ap != NULL);
        err = atf_io_write_fmt(fd, "%s\n", atf_dynstr_cstring(&r));
        if (atf_is_error(err))
            fatal_atf_error("Cannot write test case results", err);

        atf_dynstr_fini(&r);
    } else
        INV(ap == NULL);

    close(fd);
}

void
atf_tc_fail(const char *fmt, ...)
{
    va_list ap;

    PRE(current_tc != NULL);

    va_start(ap, fmt);
    write_tcr(current_tc, "failed", fmt, &ap);
    va_end(ap);

    exit(EXIT_SUCCESS);
}

void
atf_tc_pass(void)
{
    PRE(current_tc != NULL);

    write_tcr(current_tc, "passed", NULL, NULL);

    exit(EXIT_SUCCESS);
}

void
atf_tc_skip(const char *fmt, ...)
{
    va_list ap;

    PRE(current_tc != NULL);

    va_start(ap, fmt);
    write_tcr(current_tc, "skipped", fmt, &ap);
    va_end(ap);

    exit(EXIT_SUCCESS);
}
