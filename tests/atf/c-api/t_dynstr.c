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

#include <stdio.h>
#include <string.h>

#include "atf-c/dynstr.h"
#include "atf-c/macros.h"
#include "atf-c/tc.h"
#include "atf-c/tp.h"

static
void
init_head(atf_tc_t *tc)
{
    atf_tc_set_var("descr", "XXX");
}

static
void
init_body(const atf_tc_t *tc)
{
    struct atf_dynstr ad;

    atf_dynstr_init(&ad);
    atf_dynstr_fini(&ad);
}

ATF_TC(init);

static
void
init_ap_head(atf_tc_t *tc)
{
    atf_tc_set_var("descr", "XXX");
}

static
void
init_ap_body(const atf_tc_t *tc)
{
    struct atf_dynstr ad;

    atf_dynstr_init(&ad);
    atf_dynstr_fini(&ad);
}

ATF_TC(init_ap);

static
void
init_fmt_head(atf_tc_t *tc)
{
    atf_tc_set_var("descr", "XXX");
}

static
void
init_fmt_body(const atf_tc_t *tc)
{
    const char *cstr;
    struct atf_dynstr ad;

    atf_dynstr_init_fmt(&ad, "A string");
    cstr = atf_dynstr_cstring(&ad);
    ATF_CHECK(strcmp(cstr, "A string") == 0);
    atf_dynstr_fini(&ad);

    atf_dynstr_init_fmt(&ad, "A string, an int %d", 5);
    cstr = atf_dynstr_cstring(&ad);
    ATF_CHECK(strcmp(cstr, "A string, an int 5") == 0);
    atf_dynstr_fini(&ad);

    atf_dynstr_init_fmt(&ad, "A %s, an int %d", "string", 5);
    cstr = atf_dynstr_cstring(&ad);
    ATF_CHECK(strcmp(cstr, "A string, an int 5") == 0);
    atf_dynstr_fini(&ad);
}

ATF_TC(init_fmt);

static
void
init_rep_head(atf_tc_t *tc)
{
    atf_tc_set_var("descr", "XXX");
}

static
void
init_rep_body(const atf_tc_t *tc)
{
    const size_t maxlen = 1024;
    char buf[maxlen];
    size_t i;
    struct atf_dynstr ad;

    buf[0] = '\0';

    for (i = 0; i < maxlen; i++) {
        atf_dynstr_init_rep(&ad, i, 'a');
        if (strcmp(atf_dynstr_cstring(&ad), buf) != 0) {
            fprintf(stderr, "Failed at iteration %zd\n", i);
            atf_tc_fail("Failed to construct pad");
        }
        atf_dynstr_fini(&ad);

        strcat(buf, "a");
    }
}

ATF_TC(init_rep);

static
void
append_head(atf_tc_t *tc)
{
    atf_tc_set_var("descr", "XXX");
}

static
void
append_body(const atf_tc_t *tc)
{
    const size_t maxlen = 1024;
    char buf[maxlen];
    size_t i;
    struct atf_dynstr ad;

    buf[0] = '\0';

    atf_dynstr_init(&ad);
    for (i = 0; i < maxlen; i++) {
        if (strcmp(atf_dynstr_cstring(&ad), buf) != 0) {
            fprintf(stderr, "Failed at iteration %zd\n", i);
            atf_tc_fail("Failed to append character");
        }

        atf_dynstr_append(&ad, "a");
        strcat(buf, "a");
    }
    atf_dynstr_fini(&ad);
}

ATF_TC(append);

static
int
add_tcs(struct atf_tp *tp)
{
    atf_tc_init(&ATF_TC_NAME(init));
    atf_tc_init(&ATF_TC_NAME(init_ap));
    atf_tc_init(&ATF_TC_NAME(init_fmt));
    atf_tc_init(&ATF_TC_NAME(init_rep));
    atf_tc_init(&ATF_TC_NAME(append));
    atf_tp_add_tc(tp, &ATF_TC_NAME(init));
    atf_tp_add_tc(tp, &ATF_TC_NAME(init_ap));
    atf_tp_add_tc(tp, &ATF_TC_NAME(init_fmt));
    atf_tp_add_tc(tp, &ATF_TC_NAME(init_rep));
    atf_tp_add_tc(tp, &ATF_TC_NAME(append));
    return 0;
}

ATF_TP_MAIN(add_tcs);
