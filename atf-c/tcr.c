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

#include <stdbool.h>

#include "atf-c/sanity.h"
#include "atf-c/tcr.h"

/* ---------------------------------------------------------------------
 * Auxiliary types and functions.
 * --------------------------------------------------------------------- */

inline
bool
state_allows_reason(atf_tcr_state_t state)
{
    return state == atf_tcr_failed_state || state == atf_tcr_skipped_state;
}

/* ---------------------------------------------------------------------
 * The "atf_tcr" type.
 * --------------------------------------------------------------------- */

/*
 * Constants.
 */
const atf_tcr_state_t atf_tcr_passed_state = 0;
const atf_tcr_state_t atf_tcr_failed_state = 1;
const atf_tcr_state_t atf_tcr_skipped_state = 2;

/*
 * Constructors/destructors.
 */

atf_error_t
atf_tcr_init(atf_tcr_t *tcr, atf_tcr_state_t state)
{
    atf_error_t err;

    PRE(!state_allows_reason(state));

    atf_object_init(&tcr->m_object);

    tcr->m_state = state;

    err = atf_dynstr_init(&tcr->m_reason);
    if (atf_is_error(err))
        goto err_object;

    INV(!atf_is_error(err));
    return err;

err_object:
    atf_object_fini(&tcr->m_object);

    return err;
}

atf_error_t
atf_tcr_init_reason(atf_tcr_t *tcr, atf_tcr_state_t state,
                    const char *reason, ...)
{
    va_list ap;
    atf_error_t err;

    PRE(state_allows_reason(state));

    atf_object_init(&tcr->m_object);

    tcr->m_state = state;

    va_start(ap, reason);
    err = atf_dynstr_init_ap(&tcr->m_reason, reason, ap);
    va_end(ap);
    if (atf_is_error(err))
        goto err_object;

    INV(!atf_is_error(err));
    return err;

err_object:
    atf_object_fini(&tcr->m_object);

    return err;
}

void
atf_tcr_fini(atf_tcr_t *tcr)
{
    atf_dynstr_fini(&tcr->m_reason);

    atf_object_fini(&tcr->m_object);
}

/*
 * Getters. */

atf_tcr_state_t
atf_tcr_get_state(const atf_tcr_t *tcr)
{
    return tcr->m_state;
}

const atf_dynstr_t *
atf_tcr_get_reason(const atf_tcr_t *tcr)
{
    PRE(state_allows_reason(tcr->m_state));
    return &tcr->m_reason;
}
