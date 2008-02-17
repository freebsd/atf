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

#include "expand.h"
#include "io.h"
#include "sanity.h"
#include "tp.h"

static
atf_tc_t *
find_tc(atf_tp_t *tp, const char *ident)
{
    atf_tc_t *tc;
    atf_list_iter_t iter;

    tc = NULL;
    atf_list_for_each(iter, &tp->m_tcs) {
        atf_tc_t *tc2;
        tc2 = atf_list_iter_data(iter);
        if (strcmp(tc2->m_ident, ident) == 0) {
            tc = tc2;
            break;
        }
    }
    return tc;
}

static
void
match_tcs(atf_tp_t *tp, const char *name, atf_list_t *tcs)
{
    atf_list_iter_t iter;

    atf_list_for_each(iter, &tp->m_tcs) {
        atf_tc_t *tc = atf_list_iter_data(iter);

        if (atf_expand_is_glob(name)) {
            bool matches;

            atf_expand_matches_glob(name, tc->m_ident, &matches);
            if (matches) {
                atf_list_append(tcs, tc);
            }
        } else {
            if (strcmp(name, tc->m_ident) == 0) {
                atf_list_append(tcs, tc);
            }
        }
    }
}

void
atf_tp_filter_tcs(atf_tp_t *tp, atf_list_t *names, atf_list_t *tcs)
{
    atf_list_iter_t iter;

    atf_list_init(tcs);

    atf_list_for_each(iter, names) {
        const char *name = atf_list_iter_data(iter);

        match_tcs(tp, name, tcs);
    }
}

void
atf_tp_add_tc(atf_tp_t *tp, atf_tc_t *tc)
{
    PRE(find_tc(tp, tc->m_ident) == NULL);

    atf_list_append(&tp->m_tcs, tc);

    POST(find_tc(tp, tc->m_ident) != NULL);
}

void
atf_tp_init(atf_tp_t *tp)
{
    atf_object_init(&tp->m_object);

    tp->m_results_fd = STDOUT_FILENO;

    atf_list_init(&tp->m_tcs);
}

void
atf_tp_fini(atf_tp_t *tp)
{
    atf_list_iter_t iter;

    atf_list_for_each(iter, &tp->m_tcs) {
        atf_tc_t *tc = atf_list_iter_data(iter);
        atf_tc_fini(tc);
    }
    atf_list_fini(&tp->m_tcs);

    atf_object_fini(&tp->m_object);
}

void
atf_tp_set_results_fd(atf_tp_t *tp, int fd)
{
    assert(fd >= 0 && fd <= 9);
    tp->m_results_fd = fd;
}

int
atf_tp_run(atf_tp_t *tp, atf_list_t *tcnames)
{
    int code;
    size_t count;
    atf_list_t tcs;
    atf_list_iter_t iter;

    atf_tp_filter_tcs(tp, tcnames, &tcs);

    atf_io_write(tp->m_results_fd,
                 "Content-Type: application/X-atf-tcs; version=\"1\"\n\n");
    atf_io_write(tp->m_results_fd, "tcs-count: %d\n",
                 atf_list_size(&tcs));

    code = EXIT_SUCCESS;
    count = 0;
    atf_list_for_each(iter, &tcs) {
        atf_tc_t *tc = atf_list_iter_data(iter);
        atf_tcr_t tcr;

        atf_io_write(tp->m_results_fd, "tp-start: %s\n", tc->m_ident);

        tcr = atf_tc_run(tc);

        if (count < atf_list_size(&tcs)) {
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

        count++;
    }

    atf_list_fini(&tcs);

    return code;
}
