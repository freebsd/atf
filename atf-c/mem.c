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

#include <err.h>
#include <stdlib.h>

#include "atf-c/mem.h"
#include "atf-c/sanity.h"

static size_t balance = 0;
static bool initialized = false;
static bool exit_checks = true;

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

static
void
check_balance(void)
{
    PRE(initialized);

    if (exit_checks && balance > 0) {
        warnx("FATAL ERROR: Invalid balance: %zd memory regions were not "
              "freed", balance);
        abort();
    }
}

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

atf_error_t
atf_mem_alloc(void **ptr, size_t size)
{
    atf_error_t merr;
    void *m;

    PRE(initialized);
    PRE(size > 0);

    m = malloc(size); /* NO_CHECK_STYLE.c.malloc */
    if (m == NULL)
        merr = atf_no_memory_error();
    else {
        *ptr = m;
        balance++;
        merr = atf_no_error();
    }

    return merr;
}

void
atf_mem_free(void *ptr)
{
    PRE(initialized);
    PRE(balance > 0);

    free(ptr); /* NO_CHECK_STYLE.c.free */

    balance--;
}

void
atf_mem_sys_init(void)
{
    PRE(!initialized);

    initialized = true;

    if (atexit(check_balance) == -1)
        err(EXIT_FAILURE, "FATAL ERROR: Cannot initialize memory system");
}

void
atf_mem_sys_disable_exit_checks(void)
{
    PRE(initialized);

    exit_checks = false;
}
