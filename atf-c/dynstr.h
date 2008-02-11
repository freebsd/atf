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

#if !defined(ATF_C_DYNSTR_H)
#define ATF_C_DYNSTR_H

#include <stdarg.h>
#include <stddef.h>

struct atf_dynstr {
    char *ad_data;
    size_t ad_datasize;
    size_t ad_length;
};

void atf_dynstr_init(struct atf_dynstr *);
int atf_dynstr_init_ap(struct atf_dynstr *, const char *, va_list);
int atf_dynstr_init_fmt(struct atf_dynstr *, const char *, ...);
int atf_dynstr_init_rep(struct atf_dynstr *, size_t, char);
void atf_dynstr_fini(struct atf_dynstr *);

int atf_dynstr_append(struct atf_dynstr *, const char *);

const char *atf_dynstr_cstring(const struct atf_dynstr *);
size_t atf_dynstr_length(struct atf_dynstr *);

int atf_dynstr_format_ap(const char *, va_list, char **);

#endif /* ATF_C_DYNSTR_H */
