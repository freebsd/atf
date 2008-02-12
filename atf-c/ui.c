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

#include <sys/ioctl.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "dynstr.h"
#include "env.h"
#include "sanity.h"
#include "text.h"
#include "ui.h"

static
size_t
terminal_width(void)
{
    static bool done = false;
    static size_t width = 0;

    if (!done) {
        if (atf_env_has("COLUMNS")) {
            const char *cols = atf_env_get("COLUMNS");
            if (strlen(cols) > 0) {
                width = atoi(cols); /* XXX No error checking */
            }
        } else {
            struct winsize ws;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
                width = ws.ws_col;
        }

        if (width >= 80)
            width -= 5;

        done = true;
    }

    return width;
}

static
void
format_paragraph(atf_dynstr_t *dest, const char *tag, bool first,
                 bool repeat, size_t col, char *str)
{
    atf_dynstr_t pad, fullpad;

    atf_dynstr_init_rep(&pad, col - strlen(tag), ' ');
    atf_dynstr_init_rep(&fullpad, col, ' ');

    if (first || repeat) {
        atf_dynstr_append(dest, tag);
        atf_dynstr_append(dest, atf_dynstr_cstring(&pad));
    } else
        atf_dynstr_append(dest, atf_dynstr_cstring(&fullpad));

    size_t curcol = col;

    const size_t maxcol = terminal_width();

    char *last, *str2;

    str2 = strtok_r(str, " ", &last);
    while (str2 != NULL) {
        if (str != str2 && maxcol > 0 && curcol + strlen(str2) + 1 > maxcol) {
            atf_dynstr_append(dest, "\n");
            if (repeat) {
                atf_dynstr_append(dest, tag);
                atf_dynstr_append(dest, atf_dynstr_cstring(&pad));
            } else
                atf_dynstr_append(dest, atf_dynstr_cstring(&fullpad));
            curcol = col;
        } else if (str != str2) {
            atf_dynstr_append(dest, " ");
            curcol++;
        }

        atf_dynstr_append(dest, str2);
        curcol += strlen(str2);

        str2 = strtok_r(NULL, " ", &last);
    }

    atf_dynstr_fini(&fullpad);
    atf_dynstr_fini(&pad);
}

static
int
format_text_with_tag_aux(atf_dynstr_t *dest, const char *tag,
                         bool repeat, size_t col, char *str)
{
    char *last, *str2;

    PRE(col == 0 || col >= strlen(tag));

    if (col == 0)
        col = strlen(tag);

    str2 = strtok_r(str, "\n", &last);
    while (str2 != NULL) {
        format_paragraph(dest, tag, str2 == str, repeat, col, str2);
        if (last != NULL) {
            if (repeat) {
                atf_dynstr_append(dest, "\n");
                atf_dynstr_append(dest, tag);
                atf_dynstr_append(dest, "\n");
            } else
                atf_dynstr_append(dest, "\n\n");
        }

        str2 = strtok_r(NULL, "\n", &last);
    }

    return 0;
}

int
atf_ui_format_text_with_tag_ap(atf_dynstr_t *dest, const char *tag,
                               bool repeat, size_t col, const char *fmt,
                               va_list ap)
{
    int ret;
    char *src;

    ret = atf_text_format_ap(&src, fmt, ap);
    if (ret != 0)
        return ret;

    ret = format_text_with_tag_aux(dest, tag, repeat, col, src);

    free(src);

    return 0;
}

int
atf_ui_format_text_with_tag(atf_dynstr_t *dest, const char *tag,
                            bool repeat, size_t col, const char *fmt,
                            ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = atf_ui_format_text_with_tag_ap(dest, tag, repeat, col, fmt, ap);
    va_end(ap);

    return ret;
}
