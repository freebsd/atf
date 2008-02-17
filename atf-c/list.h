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

#if !defined(ATF_C_LIST_H)
#define ATF_C_LIST_H

#include <stdarg.h>

#include <atf-c/error.h>
#include <atf-c/object.h>

/* ---------------------------------------------------------------------
 * The "atf_list_iter" type.
 * --------------------------------------------------------------------- */

struct atf_list_iter {
    struct atf_list *m_list;
    void *m_entry;
};
typedef struct atf_list_iter atf_list_iter_t;

/* Getters. */
void *atf_list_iter_data(const atf_list_iter_t);
atf_list_iter_t atf_list_iter_next(const atf_list_iter_t);

/* Operators. */
bool atf_equal_list_iter_list_iter(const atf_list_iter_t,
                                   const atf_list_iter_t);

/* ---------------------------------------------------------------------
 * The "atf_list" type.
 * --------------------------------------------------------------------- */

struct atf_list {
    atf_object_t m_object;

    atf_list_iter_t m_begin;
    atf_list_iter_t m_end;

    size_t m_size;
};
typedef struct atf_list atf_list_t;

/* Constructors and destructors */
atf_error_t atf_list_init(atf_list_t *);
void atf_list_fini(atf_list_t *);

/* Getters. */
atf_list_iter_t atf_list_begin(atf_list_t *);
atf_list_iter_t atf_list_end(atf_list_t *);
size_t atf_list_size(const atf_list_t *);

/* Modifiers. */
atf_error_t atf_list_append(atf_list_t *, void *);

/* Macros. */
#define atf_list_for_each(iter, list) \
    for (iter = atf_list_begin(list); \
         !atf_equal_list_iter_list_iter((iter), atf_list_end(list)); \
         iter = atf_list_iter_next(iter))

#endif /* ATF_C_LIST_H */
