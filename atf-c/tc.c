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

#include "io.h"
#include "sanity.h"
#include "tc.h"
#include "text.h"

static void body_parent(const atf_tc_t *, pid_t, atf_tcr_t *);
static void body_child(const atf_tc_t *) __attribute__((noreturn));

/* ---------------------------------------------------------------------
 * The "atf_tcr" type.
 * --------------------------------------------------------------------- */

const int atf_tcr_passed = 0;
const int atf_tcr_failed = 1;
const int atf_tcr_skipped = 2;

inline
bool
status_allows_reason(int status)
{
    return status == atf_tcr_failed || status == atf_tcr_skipped;
}

void
atf_tcr_init(atf_tcr_t *tcr, int status)
{
    atf_object_init(&tcr->m_object);

    PRE(!status_allows_reason(status));
    tcr->m_status = status;
    atf_dynstr_init(&tcr->m_reason);
}

int
atf_tcr_init_reason(atf_tcr_t *tcr, int status, const char *reason, ...)
{
    int ret = 0;
    va_list ap;

    PRE(status_allows_reason(status));

    atf_object_init(&tcr->m_object);

    tcr->m_status = status;
    va_start(ap, reason);
    ret = atf_dynstr_init_ap(&tcr->m_reason, reason, ap);
    va_end(ap);

    return ret;
}

void
atf_tcr_fini(atf_tcr_t *tcr)
{
    atf_dynstr_fini(&tcr->m_reason);

    atf_object_fini(&tcr->m_object);
}

int
atf_tcr_get_status(const atf_tcr_t *tcr)
{
    return tcr->m_status;
}

const char *
atf_tcr_get_reason(const atf_tcr_t *tcr)
{
    PRE(status_allows_reason(tcr->m_status));
    return atf_dynstr_cstring(&tcr->m_reason);
}

/* ---------------------------------------------------------------------
 * The "atf_tc" type.
 * --------------------------------------------------------------------- */

void
atf_tc_init(atf_tc_t *tc)
{
    atf_object_init(&tc->m_object);

    tc->m_workdir = NULL;
}

void
atf_tc_fini(atf_tc_t *tc)
{
    atf_object_fini(&tc->m_object);
}

atf_tcr_t
atf_tc_run(atf_tc_t *tc)
{
    char workdir[1024];
    pid_t pid;
    atf_tcr_t tcr;

    strcpy(workdir, "/tmp/atf.XXXXXX");
    if (mkdtemp(workdir) == NULL) {
    }

    tc->m_workdir = workdir;
    pid = fork();
    if (pid == -1) {
    } else if (pid == 0) {
        body_child(tc);
        UNREACHABLE;
    } else {
        body_parent(tc, pid, &tcr);
    }
    tc->m_workdir = NULL;

    return tcr;
}

void
atf_tc_set_var(const char *var, const char *value, ...)
{
}

/*
 * Parent-only stuff.
 */

static
void
body_parent(const atf_tc_t *tc, pid_t pid, atf_tcr_t *tcr)
{
    int fd, ws;
    atf_dynstr_t status, reason;

    if (waitpid(pid, &ws, 0) == -1) {
        int err = errno;
        atf_tcr_init_reason(tcr, atf_tcr_failed,
                            "Error waiting for child process: %s\n", err);
        return;
    }

    {
        char *filename;
        int err;

        atf_text_format(&filename, "%s/tc-result", tc->m_workdir);
        fd = open(filename, O_RDONLY);
        err = errno;
        free(filename);

        if (fd == -1) {
            atf_tcr_init_reason(tcr, atf_tcr_failed,
                                "Error opening status file: %s\n", err);
            return;
        }
    }

    atf_dynstr_init(&status);
    atf_dynstr_init(&reason);

    atf_io_readline(fd, &status);
    if (atf_dynstr_compare(&status, "passed") == 0)
        atf_tcr_init(tcr, atf_tcr_passed);
    else {
        atf_io_readline(fd, &reason);
        if (atf_dynstr_compare(&status, "failed") == 0)
            atf_tcr_init_reason(tcr, atf_tcr_failed, "%s",
                                atf_dynstr_cstring(&reason));
        else if (atf_dynstr_compare(&status, "skipped") == 0)
            atf_tcr_init_reason(tcr, atf_tcr_skipped, "%s",
                                atf_dynstr_cstring(&reason));
        else
            UNREACHABLE;
    }

    atf_dynstr_fini(&reason);
    atf_dynstr_fini(&status);

    close(fd);
}

/*
 * Child-only stuff.
 */

static const atf_tc_t *current_tc = NULL;

static
void
body_child(const atf_tc_t *tc)
{
    atf_disable_exit_checks();

    current_tc = tc;
    tc->m_body(tc);
    atf_tc_pass();
    UNREACHABLE;
}

static
void
write_tcr(const atf_tc_t *tc, const char *status, char *reason)
{
    char *filename;
    FILE *f;

    atf_text_format(&filename, "%s/tc-result", tc->m_workdir);

    f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "Failed to open results file");
        abort();
    }
    fprintf(f, "%s\n", status);
    if (reason != NULL)
        fprintf(f, "%s\n", reason);
    fclose(f);

    free(filename);
}

void
atf_tc_fail(const char *fmt, ...)
{
    char *reason;
    va_list ap;

    va_start(ap, fmt);
    atf_text_format(&reason, fmt, ap);
    va_end(ap);

    write_tcr(current_tc, "failed", reason);

    free(reason);

    exit(EXIT_SUCCESS);
}

void
atf_tc_pass(void)
{
    write_tcr(current_tc, "passed", NULL);

    exit(EXIT_SUCCESS);
}

void
atf_tc_skip(const char *fmt, ...)
{
    char *reason;
    va_list ap;

    va_start(ap, fmt);
    atf_text_format(&reason, fmt, ap);
    va_end(ap);

    write_tcr(current_tc, "skipped", reason);

    free(reason);

    exit(EXIT_SUCCESS);
}
