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
struct atf_tc *
find_tc(const struct atf_tp *tp, const char *ident)
{
    struct atf_tc *tc;

    tc = NULL;
    TAILQ_FOREACH(tc, &tp->atp_tcs, atc_link) {
        if (strcmp(tc->atc_ident, ident) == 0)
            break;
    }
    return tc;
}

void
atf_tp_add_tc(struct atf_tp *tp, struct atf_tc *tc)
{
    PRE(find_tc(tp, tc->atc_ident) == NULL);

    TAILQ_INSERT_TAIL(&tp->atp_tcs, tc, atc_link);
    tp->atp_tcs_count++;

    POST(find_tc(tp, tc->atc_ident) != NULL);
}

void
atf_tp_init(struct atf_tp *tp)
{
    tp->atp_results_fd = STDOUT_FILENO;
    TAILQ_INIT(&tp->atp_tcs);
    tp->atp_tcs_count = 0;
}

void
atf_tp_set_results_fd(struct atf_tp *tp, int fd)
{
    assert(fd >= 0 && fd <= 9);
    tp->atp_results_fd = fd;
}

int
atf_tp_run(struct atf_tp *tp)
{
    int code;
    struct atf_tc *tc;

    atf_io_write(tp->atp_results_fd,
                 "Content-Type: application/X-atf-tcs; version=\"1\"\n\n");
    atf_io_write(tp->atp_results_fd, "tcs-count: %d\n", tp->atp_tcs_count);

    code = EXIT_SUCCESS;
    TAILQ_FOREACH(tc, &tp->atp_tcs, atc_link) {
        struct atf_tcr tcr;

        atf_io_write(tp->atp_results_fd, "tp-start: %s\n", tc->atc_ident);

        atf_tc_run(tc, &tcr);

        if (tc != TAILQ_LAST(&tp->atp_tcs, atf_tc_list)) {
            fprintf(stdout, "__atf_tc_separator__\n");
            fprintf(stderr, "__atf_tc_separator__\n");
        }
        fflush(stdout);
        fflush(stderr);

        {
            int status = atf_tcr_get_status(&tcr);
            if (status == atf_tcr_passed) {
                atf_io_write(tp->atp_results_fd, "tp-end: %s, passed\n",
                             tc->atc_ident);
            } else if (status == atf_tcr_failed) {
                atf_io_write(tp->atp_results_fd, "tp-end: %s, failed, %s\n",
                             tc->atc_ident, atf_tcr_get_reason(&tcr));
            } else if (status == atf_tcr_skipped) {
                atf_io_write(tp->atp_results_fd, "tp-end: %s, skipped, %s\n",
                             tc->atc_ident, atf_tcr_get_reason(&tcr));
            } else
                UNREACHABLE;
        }

        atf_tcr_fini(&tcr);
    }

    return code;
}
