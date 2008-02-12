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

#include "dynstr.h"
#include "sanity.h"

static
int
resize(atf_dynstr_t *ad, size_t newsize)
{
    char *newdata;

    PRE(newsize > ad->m_datasize);

    newdata = (char *)malloc(newsize);
    if (newdata == NULL)
        return ENOMEM;

    strcpy(newdata, ad->m_data);
    free(ad->m_data);
    ad->m_data = newdata;
    ad->m_datasize = newsize;

    return 0;
}

void
atf_dynstr_init(atf_dynstr_t *ad)
{
    atf_object_init(&ad->m_object);

    ad->m_data = (char *)malloc(1);
    ad->m_data[0] = '\0';
    ad->m_datasize = 1;
    ad->m_length = 0;
}

int
atf_dynstr_init_rep(atf_dynstr_t *ad, size_t len, char ch)
{
    atf_object_init(&ad->m_object);

    ad->m_data = (char *)malloc(len + 1);
    ad->m_datasize = len + 1;
    if (ad->m_data == NULL)
        return ENOMEM;
    memset(ad->m_data, ch, len);
    ad->m_data[len] = '\0';
    ad->m_length = len;
    return 0;
}

int
atf_dynstr_init_ap(atf_dynstr_t *ad, const char *fmt, va_list ap)
{
    atf_object_init(&ad->m_object);

    ad->m_datasize = strlen(fmt) * 2 + 1;
    for (;;) {
        ad->m_data = (char *)malloc(ad->m_datasize);
        if (ad->m_data == NULL)
            return ENOMEM;
        ad->m_length = vsnprintf(ad->m_data, ad->m_datasize, fmt, ap);
        if (ad->m_length < ad->m_datasize)
            break;

        free(ad->m_data);
        ad->m_datasize *= 2;
    }

    return 0;
}

int
atf_dynstr_init_fmt(atf_dynstr_t *ad, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = atf_dynstr_init_ap(ad, fmt, ap);
    va_end(ap);

    return ret;
}

void
atf_dynstr_fini(atf_dynstr_t *ad)
{
    if (ad->m_data != NULL)
        free(ad->m_data);

    atf_object_fini(&ad->m_object);
}

int
atf_dynstr_append(atf_dynstr_t *ad, const char *str)
{
    int err;
    size_t newlen = ad->m_length + strlen(str);

    if (newlen >= ad->m_datasize) {
        err = resize(ad, newlen + 1);
        if (err != 0)
            return err;
    }

    strcat(ad->m_data, str);
    ad->m_length += strlen(str);

    return 0;
}

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

int
atf_dynstr_format_ap(const char *fmt, va_list ap, char **dest)
{
    int ret;
    atf_dynstr_t tmp;

    ret = atf_dynstr_init_ap(&tmp, fmt, ap);
    if (ret != 0)
        return ret;

    *dest = tmp.m_data;
    tmp.m_data = NULL;
    atf_dynstr_fini(&tmp);

    return 0;
}
