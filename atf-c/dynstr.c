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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atf-c/dynstr.h"
#include "atf-c/sanity.h"
#include "atf-c/text.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

static
atf_error_t
resize(atf_dynstr_t *ad, size_t newsize)
{
    char *newdata;
    atf_error_t err;

    PRE(newsize > ad->m_datasize);

    newdata = (char *)malloc(newsize);
    if (newdata == NULL) {
        err = atf_no_memory_error();
    } else {
        strcpy(newdata, ad->m_data);
        free(ad->m_data);
        ad->m_data = newdata;
        ad->m_datasize = newsize;
        err = atf_no_error();
    }

    return err;
}

atf_error_t
prepend_or_append(atf_dynstr_t *ad, const char *fmt, va_list ap,
                  bool prepend)
{
    char *aux;
    atf_error_t err;
    size_t newlen;

    err = atf_text_format_ap(&aux, fmt, ap);
    if (atf_is_error(err))
        goto out;
    newlen = ad->m_length + strlen(aux);

    if (newlen + sizeof(char) > ad->m_datasize) {
        err = resize(ad, newlen + sizeof(char));
        if (atf_is_error(err))
            goto out_free;
    }

    if (prepend) {
        memmove(ad->m_data + strlen(aux), ad->m_data, ad->m_length + 1);
        memcpy(ad->m_data, aux, strlen(aux));
    } else
        strcpy(ad->m_data + ad->m_length, aux);
    ad->m_length = newlen;
    err = atf_no_error();

out_free:
    free(aux);
out:
    return err;
}

/* ---------------------------------------------------------------------
 * The "atf_dynstr" type.
 * --------------------------------------------------------------------- */

/*
 * Constructors and destructors.
 */

atf_error_t
atf_dynstr_init(atf_dynstr_t *ad)
{
    atf_error_t err;

    atf_object_init(&ad->m_object);

    ad->m_data = (char *)malloc(sizeof(char));
    if (ad->m_data == NULL) {
        err = atf_no_memory_error();
    } else {
        ad->m_data[0] = '\0';
        ad->m_datasize = 1;
        ad->m_length = 0;
        err = atf_no_error();
    }

    return err;
}

atf_error_t
atf_dynstr_init_ap(atf_dynstr_t *ad, const char *fmt, va_list ap)
{
    atf_error_t err;

    atf_object_init(&ad->m_object);

    ad->m_datasize = strlen(fmt) + 1;
    ad->m_length = 0;

    err = atf_no_error();
    do {
        ad->m_datasize *= 2;
        ad->m_data = (char *)malloc(ad->m_datasize);
        if (ad->m_data == NULL) {
            err = atf_no_memory_error();
        } else {
            int ret = vsnprintf(ad->m_data, ad->m_datasize, fmt, ap);
            if (ret < 0) {
                err = atf_libc_error(errno, "Cannot format string");
            } else {
                if (ret >= ad->m_datasize) {
                    free(ad->m_data);
                    ad->m_data = NULL;
                }
                ad->m_length = ret;
            }
        }
    } while (!atf_is_error(err) && ad->m_length >= ad->m_datasize);

    POST(atf_is_error(err) || ad->m_data != NULL);
    return err;
}

atf_error_t
atf_dynstr_init_fmt(atf_dynstr_t *ad, const char *fmt, ...)
{
    va_list ap;
    atf_error_t err;

    va_start(ap, fmt);
    err = atf_dynstr_init_ap(ad, fmt, ap);
    va_end(ap);

    return err;
}

atf_error_t
atf_dynstr_init_rep(atf_dynstr_t *ad, size_t len, char ch)
{
    atf_error_t err;

    atf_object_init(&ad->m_object);

    if (len == SIZE_MAX)
        err = atf_no_memory_error();
    else {
        ad->m_datasize = len + sizeof(char);
        ad->m_data = (char *)malloc(ad->m_datasize);
        if (ad->m_data == NULL) {
            err = atf_no_memory_error();
        } else {
            memset(ad->m_data, ch, len);
            ad->m_data[len] = '\0';
            ad->m_length = len;
            err = atf_no_error();
        }
    }

    return err;
}

void
atf_dynstr_fini(atf_dynstr_t *ad)
{
    INV(ad->m_data != NULL);
    free(ad->m_data);

    atf_object_fini(&ad->m_object);
}

char *
atf_dynstr_fini_disown(atf_dynstr_t *ad)
{
    atf_object_fini(&ad->m_object);

    INV(ad->m_data != NULL);
    return ad->m_data;
}

/*
 * Getters.
 */

const char *
atf_dynstr_cstring(const atf_dynstr_t *ad)
{
    return ad->m_data;
}

size_t
atf_dynstr_length(atf_dynstr_t *ad)
{
    return ad->m_length;
}

/*
 * Modifiers.
 */

atf_error_t
atf_dynstr_append_ap(atf_dynstr_t *ad, const char *fmt, va_list ap)
{
    return prepend_or_append(ad, fmt, ap, false);
}

atf_error_t
atf_dynstr_append_fmt(atf_dynstr_t *ad, const char *fmt, ...)
{
    va_list ap;
    atf_error_t err;

    va_start(ap, fmt);
    err = prepend_or_append(ad, fmt, ap, false);
    va_end(ap);

    return err;
}

void
atf_dynstr_clear(atf_dynstr_t *ad)
{
    ad->m_data[0] = '\0';
    ad->m_length = 0;
}

atf_error_t
atf_dynstr_prepend_ap(atf_dynstr_t *ad, const char *fmt, va_list ap)
{
    return prepend_or_append(ad, fmt, ap, true);
}

atf_error_t
atf_dynstr_prepend_fmt(atf_dynstr_t *ad, const char *fmt, ...)
{
    va_list ap;
    atf_error_t err;

    va_start(ap, fmt);
    err = prepend_or_append(ad, fmt, ap, true);
    va_end(ap);

    return err;
}

/*
 * Operators.
 */

bool
atf_equal_dynstr_cstring(const atf_dynstr_t *ad, const char *str)
{
    return strcmp(ad->m_data, str) == 0;
}
