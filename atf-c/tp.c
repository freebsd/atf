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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "io.h"
#include "sanity.h"
#include "tp.h"

static
atf_tc_t *
find_tc(const atf_tp_t *tp, const char *ident)
{
    atf_tc_t *tc;

    tc = NULL;
    TAILQ_FOREACH(tc, &tp->m_tcs, m_link) {
        if (strcmp(tc->m_ident, ident) == 0)
            break;
    }
    return tc;
}

void
atf_tp_add_tc(atf_tp_t *tp, atf_tc_t *tc)
{
    PRE(find_tc(tp, tc->m_ident) == NULL);

    TAILQ_INSERT_TAIL(&tp->m_tcs, tc, m_link);
    tp->m_tcs_count++;

    POST(find_tc(tp, tc->m_ident) != NULL);
}

void
atf_tp_init(atf_tp_t *tp)
{
    atf_object_init(&tp->m_object);

    tp->m_results_fd = STDOUT_FILENO;
    TAILQ_INIT(&tp->m_tcs);
    tp->m_tcs_count = 0;
}

void
atf_tp_fini(atf_tp_t *tp)
{
    atf_tc_t *tc;

    TAILQ_FOREACH(tc, &tp->m_tcs, m_link) {
        atf_tc_fini(tc);
    }

    atf_object_fini(&tp->m_object);
}

void
atf_tp_set_results_fd(atf_tp_t *tp, int fd)
{
    assert(fd >= 0 && fd <= 9);
    tp->m_results_fd = fd;
}

int
atf_tp_run(atf_tp_t *tp)
{
    int code;
    atf_tc_t *tc;

    atf_io_write(tp->m_results_fd,
                 "Content-Type: application/X-atf-tcs; version=\"1\"\n\n");
    atf_io_write(tp->m_results_fd, "tcs-count: %d\n", tp->m_tcs_count);

    code = EXIT_SUCCESS;
    TAILQ_FOREACH(tc, &tp->m_tcs, m_link) {
        atf_tcr_t tcr;

        atf_io_write(tp->m_results_fd, "tp-start: %s\n", tc->m_ident);

        tcr = atf_tc_run(tc);

        if (tc != TAILQ_LAST(&tp->m_tcs, atf_tc_list)) {
            fprintf(stdout, "__atf_tc_separator__\n");
            fprintf(stderr, "__atf_tc_separator__\n");
        }
        fflush(stdout);
        fflush(stderr);

        {
            int status = atf_tcr_get_status(&tcr);
            if (status == atf_tcr_passed) {
                atf_io_write(tp->m_results_fd, "tp-end: %s, passed\n",
                             tc->m_ident);
            } else if (status == atf_tcr_failed) {
                atf_io_write(tp->m_results_fd, "tp-end: %s, failed, %s\n",
                             tc->m_ident, atf_tcr_get_reason(&tcr));
                code = EXIT_FAILURE;
            } else if (status == atf_tcr_skipped) {
                atf_io_write(tp->m_results_fd, "tp-end: %s, skipped, %s\n",
                             tc->m_ident, atf_tcr_get_reason(&tcr));
                code = EXIT_FAILURE;
            } else
                UNREACHABLE;
        }

        atf_tcr_fini(&tcr);
    }

    return code;
}
