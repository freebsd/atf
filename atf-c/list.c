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

#include <stdlib.h>

#include "atf-c/mem.h"
#include "atf-c/list.h"
#include "atf-c/sanity.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

struct list_entry {
    struct list_entry *m_prev;
    struct list_entry *m_next;
    void *m_object;
};

static
atf_list_citer_t
entry_to_citer(const atf_list_t *l, const struct list_entry *le)
{
    atf_list_citer_t iter;
    iter.m_list = l;
    iter.m_entry = le;
    return iter;
}

static
atf_list_iter_t
entry_to_iter(atf_list_t *l, struct list_entry *le)
{
    atf_list_iter_t iter;
    iter.m_list = l;
    iter.m_entry = le;
    return iter;
}

static
atf_error_t
new_entry(struct list_entry **lep, void *object)
{
    atf_error_t err;
    struct list_entry *le;

    err = atf_mem_alloc((void **)&le, sizeof(*le));
    if (!atf_is_error(err)) {
        le->m_prev = le->m_next = NULL;
        le->m_object = object;
        *lep = le;
    }

    return err;
}

static
atf_error_t
new_entry_and_link(struct list_entry **lep, void *object,
                   struct list_entry *prev, struct list_entry *next)
{
    atf_error_t err;
    struct list_entry *le;

    err = new_entry(&le, object);
    if (!atf_is_error(err)) {
        le->m_prev = prev;
        le->m_next = next;

        prev->m_next = le;
        next->m_prev = le;

        *lep = le;
    }

    return err;
}

/* ---------------------------------------------------------------------
 * The "atf_list_citer" type.
 * --------------------------------------------------------------------- */

/*
 * Getters.
 */

const void *
atf_list_citer_data(const atf_list_citer_t citer)
{
    const struct list_entry *le = citer.m_entry;
    PRE(le != NULL);
    return le->m_object;
}

atf_list_citer_t
atf_list_citer_next(const atf_list_citer_t citer)
{
    const struct list_entry *le = citer.m_entry;
    atf_list_citer_t newciter;

    PRE(le != NULL);

    newciter = citer;
    newciter.m_entry = le->m_next;

    return newciter;
}

bool
atf_equal_list_citer_list_citer(const atf_list_citer_t i1,
                                const atf_list_citer_t i2)
{
    return i1.m_list == i2.m_list && i1.m_entry == i2.m_entry;
}

/* ---------------------------------------------------------------------
 * The "atf_list_iter" type.
 * --------------------------------------------------------------------- */

/*
 * Getters.
 */

void *
atf_list_iter_data(const atf_list_iter_t iter)
{
    const struct list_entry *le = iter.m_entry;
    PRE(le != NULL);
    return le->m_object;
}

atf_list_iter_t
atf_list_iter_next(const atf_list_iter_t iter)
{
    const struct list_entry *le = iter.m_entry;
    atf_list_iter_t newiter;

    PRE(le != NULL);

    newiter = iter;
    newiter.m_entry = le->m_next;

    return newiter;
}

bool
atf_equal_list_iter_list_iter(const atf_list_iter_t i1,
                              const atf_list_iter_t i2)
{
    return i1.m_list == i2.m_list && i1.m_entry == i2.m_entry;
}

/* ---------------------------------------------------------------------
 * The "atf_list" type.
 * --------------------------------------------------------------------- */

/*
 * Constructors and destructors.
 */

atf_error_t
atf_list_init(atf_list_t *l)
{
    atf_error_t err;
    struct list_entry *lebeg, *leend;

    err = new_entry(&lebeg, NULL);
    if (atf_is_error(err)) {
        return err;
    }

    err = new_entry(&leend, NULL);
    if (atf_is_error(err)) {
        atf_mem_free(lebeg);
        return err;
    }

    lebeg->m_next = leend;
    lebeg->m_prev = NULL;

    leend->m_next = NULL;
    leend->m_prev = lebeg;

    l->m_size = 0;
    l->m_begin = lebeg;
    l->m_end = leend;

    atf_object_init(&l->m_object);

    return atf_no_error();
}

void
atf_list_fini(atf_list_t *l)
{
    struct list_entry *le;
    size_t freed;

    le = (struct list_entry *)l->m_begin;
    freed = 0;
    while (le != NULL) {
        struct list_entry *lenext;

        lenext = le->m_next;
        atf_mem_free(le);
        le = lenext;

        freed++;
    }
    INV(freed == l->m_size + 2);

    atf_object_fini(&l->m_object);
}

/*
 * Getters.
 */

atf_list_iter_t
atf_list_begin(atf_list_t *l)
{
    struct list_entry *le = l->m_begin;
    return entry_to_iter(l, le->m_next);
}

atf_list_citer_t
atf_list_begin_c(const atf_list_t *l)
{
    const struct list_entry *le = l->m_begin;
    return entry_to_citer(l, le->m_next);
}

atf_list_iter_t
atf_list_end(atf_list_t *l)
{
    return entry_to_iter(l, l->m_end);
}

atf_list_citer_t
atf_list_end_c(const atf_list_t *l)
{
    return entry_to_citer(l, l->m_end);
}

size_t
atf_list_size(const atf_list_t *l)
{
    return l->m_size;
}

/*
 * Modifiers.
 */

atf_error_t
atf_list_append(atf_list_t *l, void *data)
{
    struct list_entry *le, *next, *prev;
    atf_error_t err;

    next = (struct list_entry *)l->m_end;
    prev = next->m_prev;
    err = new_entry_and_link(&le, data, prev, next);
    if (!atf_is_error(err))
        l->m_size++;

    return err;
}
