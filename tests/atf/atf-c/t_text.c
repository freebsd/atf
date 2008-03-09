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
#include <string.h>

#include <atf-c.h>

#include "atf-c/text.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

#define CE(stm) ATF_CHECK(!atf_is_error(stm))

/* ---------------------------------------------------------------------
 * Test cases for the free functions.
 * --------------------------------------------------------------------- */

static
atf_error_t
word_acum(const char *word, void *data)
{
    char *acum = data;

    strcat(acum, word);

    return atf_no_error();
}

static
atf_error_t
word_count(const char *word, void *data)
{
    size_t *counter = data;

    (*counter)++;

    return atf_no_error();
}

ATF_TC(for_each_word);
ATF_TC_HEAD(for_each_word, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_text_for_each_word"
                   "function");
}
ATF_TC_BODY(for_each_word, tc)
{
    size_t cnt;
    char acum[1024];

    cnt = 0;
    strcpy(acum, "");
    CE(atf_text_for_each_word("1 2 3", " ", word_count, &cnt));
    CE(atf_text_for_each_word("1 2 3", " ", word_acum, acum));
    ATF_CHECK(cnt == 3);
    ATF_CHECK(strcmp(acum, "123") == 0);

    cnt = 0;
    strcpy(acum, "");
    CE(atf_text_for_each_word("1 2 3", ".", word_count, &cnt));
    CE(atf_text_for_each_word("1 2 3", ".", word_acum, acum));
    ATF_CHECK(cnt == 1);
    ATF_CHECK(strcmp(acum, "1 2 3") == 0);

    cnt = 0;
    strcpy(acum, "");
    CE(atf_text_for_each_word("1 2 3 4 5", " ", word_count, &cnt));
    CE(atf_text_for_each_word("1 2 3 4 5", " ", word_acum, acum));
    ATF_CHECK(cnt == 5);
    ATF_CHECK(strcmp(acum, "12345") == 0);

    cnt = 0;
    strcpy(acum, "");
    CE(atf_text_for_each_word("1 2.3.4 5", " .", word_count, &cnt));
    CE(atf_text_for_each_word("1 2.3.4 5", " .", word_acum, acum));
    ATF_CHECK(cnt == 5);
    ATF_CHECK(strcmp(acum, "12345") == 0);
}

ATF_TC(format);
ATF_TC_HEAD(format, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the construction of free-form "
                   "strings using a variable parameters list");
}
ATF_TC_BODY(format, tc)
{
    char *str;
    atf_error_t err;

    err = atf_text_format(&str, "%s %s %d", "Test", "string", 1);
    ATF_CHECK(!atf_is_error(err));
    ATF_CHECK(strcmp(str, "Test string 1") == 0);
    free(str);
}

static
void
format_ap(char **dest, const char *fmt, ...)
{
    va_list ap;
    atf_error_t err;

    va_start(ap, fmt);
    err = atf_text_format_ap(dest, fmt, ap);
    va_end(ap);

    ATF_CHECK(!atf_is_error(err));
}

ATF_TC(format_ap);
ATF_TC_HEAD(format_ap, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the construction of free-form "
                   "strings using a va_list argument");
}
ATF_TC_BODY(format_ap, tc)
{
    char *str;

    format_ap(&str, "%s %s %d", "Test", "string", 1);
    ATF_CHECK(strcmp(str, "Test string 1") == 0);
    free(str);
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, for_each_word);
    ATF_TP_ADD_TC(tp, format);
    ATF_TP_ADD_TC(tp, format_ap);

    return atf_no_error();
}
