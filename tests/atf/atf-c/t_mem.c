/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2007, 2008 The NetBSD Foundation, Inc.
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

#include <limits.h>
#include <stdio.h>

#include <atf-c.h>

#include "atf-c/mem.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

#define CE(stm) ATF_CHECK(!atf_is_error(stm))

/* ---------------------------------------------------------------------
 * Test cases for the free functions.
 * --------------------------------------------------------------------- */

ATF_TC(alloc);
ATF_TC_HEAD(alloc, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the atf_mem_alloc function");
}
ATF_TC_BODY(alloc, tc)
{
    printf("Allocating SIZE_MAX should fail\n");
    {
        atf_error_t err;
        void *mem;

        err = atf_mem_alloc(&mem, SIZE_MAX);
        ATF_CHECK(atf_is_error(err));
        atf_error_free(err);
    }

    printf("Allocating and releasing 4096 chunks\n");
    {
        int i;
        void *mem;

        for (i = 1; i < 4096; i++) {
            CE(atf_mem_alloc(&mem, i));
            atf_mem_free(mem);
        }
    }

    printf("Allocating 4096 chunks and releasing at the end\n");
    {
        int i;
        void *chunks[4096];

        for (i = 1; i < 4096; i++)
            CE(atf_mem_alloc(&chunks[i - 1], i));

        for (i = 1; i < 4096; i++)
            atf_mem_free(chunks[i - 1]);
    }
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, alloc);

    return atf_no_error();
}
