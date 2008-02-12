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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dynstr.h"
#include "object.h"
#include "sanity.h"
#include "tp.h"
#include "ui.h"

static void usage(void);
static void usage_error(const char *, ...);

static
int
print_tag_ap(const char *tag, bool repeat, const char *fmt, va_list ap)
{
    int ret;
    atf_dynstr_t dest;

    atf_dynstr_init(&dest);

    if (atf_ui_format_text_with_tag_ap(&dest, tag, repeat, 0, fmt, ap) != 0) {
        ret = 0;
        goto out;
    }

    ret = printf("%s\n", atf_dynstr_cstring(&dest));

out:
    atf_dynstr_fini(&dest);
    return ret;
}

static
int
print_tag(const char *tag, bool repeat, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = print_tag_ap(tag, repeat, fmt, ap);
    va_end(ap);

    return ret;
}

static
int
print(const char *msg)
{
    return print_tag("", false, msg);
}

static
void
print_error_ap(const char *fmt, va_list ap)
{
    atf_dynstr_t tag;

    if (atf_dynstr_init_fmt(&tag, "%s: ERROR: ", getprogname()) != 0)
        atf_dynstr_init(&tag);

    print_tag_ap(atf_dynstr_cstring(&tag), true, fmt, ap);

    atf_dynstr_fini(&tag);
}

static
void
print_error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print_error(fmt, ap);
    va_end(ap);
}

static
void
usage(void)
{
    print_tag("Usage: ", false,
              "%s [options] [test_case1 [.. test_caseN]]",
              getprogname());
    printf("\n");
    print("This is an independent atf test program.");
    printf("\n");
    print("Available options:");
    print_tag("    -h              ", false,
              "Shows this help message");
    print_tag("    -l              ", false,
              "List test cases and their purpose");
    print_tag("    -r fd           ", false,
              "The file descriptor to which the test program "
              "will send the results of the test cases");
    print_tag("    -s srcdir       ", false,
              "Directory where the test's data files are "
              "located");
    print_tag("    -v var=value    ", false,
              "Sets the configuration variable `var' to `value'");
    printf("\n");
    print("For more details please see atf-test-program(1) and atf(7).");
}

static
void
usage_error(const char *fmt, ...)
{
    va_list ap;
    atf_dynstr_t tag;

    va_start(ap, fmt);
    print_error_ap(fmt, ap);
    va_end(ap);

    atf_dynstr_init_fmt(&tag, "%s: ", getprogname());
    print_tag(atf_dynstr_cstring(&tag), true,
              "Type `%s -h' for more details.", getprogname());
    atf_dynstr_fini(&tag);
}

static
bool
parse_rflag(const char *arg, int *value)
{
    if (strlen(arg) != 1 || !isdigit(arg[0])) {
        usage_error("Invalid value for -r; must be a single digit.");
        return false;
    }

    *value = arg[0] - '0';

    return true;
}

int
atf_tp_main(int argc, char **argv, int (*add_tcs_hook)(atf_tp_t *))
{
    bool lflag;
    int ch, ret;
    atf_tp_t tp;

    atf_init_objects();

    atf_tp_init(&tp);

    lflag = false;
    while ((ch = getopt(argc, argv, ":hlr:s:v:")) != -1) {
        switch (ch) {
        case 'h':
            usage();
            ret = EXIT_SUCCESS;
            goto out;

        case 'l':
            lflag = true;
            break;

        case 'r':
            {
                int v;
                if (!parse_rflag(optarg, &v)) {
                    ret = EXIT_FAILURE;
                    goto out;
                }
                atf_tp_set_results_fd(&tp, v);
            }
            break;

        case 's':
            break;

        case 'v':
            usage_error("foo");
            ret = EXIT_FAILURE;
            goto out;

        case ':':
            break;

        case '?':
        default:
            usage_error("Unknown option -%c.", optopt);
            ret = EXIT_FAILURE;
            goto out;
        }
    }
    argc -= optind;
    argv += optind;

    add_tcs_hook(&tp); /* XXX Handle error */

    ret = atf_tp_run(&tp);

out:
    atf_tp_fini(&tp);

    return ret;
}
