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

#include "sanity.h"
#include "tp.h"
#include "ui.h"

static void usage(void) __attribute__((noreturn));
static void usage_error(const char *, ...) __attribute__((noreturn));

static
void
usage(void)
{
    atf_ui_print_fmt_with_tag("Usage: ", false,
                       "%s [options] [test_case1 [.. test_caseN]]",
                       getprogname());
    printf("\n");
    atf_ui_print_fmt("This is an independent atf test program.");
    printf("\n");
    atf_ui_print_fmt("Available options:");
    atf_ui_print_fmt_with_tag("    -h              ", false,
                       "Shows this help message");
    atf_ui_print_fmt_with_tag("    -l              ", false,
                       "List test cases and their purpose");
    atf_ui_print_fmt_with_tag("    -r fd           ", false,
                       "The file descriptor to which the test program "
                       "will send the results of the test cases");
    atf_ui_print_fmt_with_tag("    -s srcdir       ", false,
                       "Directory where the test's data files are "
                       "located");
    atf_ui_print_fmt_with_tag("    -v var=value    ", false,
                       "Sets the configuration variable `var' to `value'");
    printf("\n");
    atf_ui_print_fmt("For more details please see atf-test-program(1).");
    printf("\n");
    exit(EXIT_SUCCESS);
}

static
void
usage_error(const char *fmt, ...)
{
    char tag[512];
    snprintf(tag, sizeof(tag), "%s: ", getprogname());
    atf_ui_print_fmt_with_tag(tag, true, "ERROR: ");
    atf_ui_print_fmt_with_tag(tag, true, "Type `%s -h' for more details.",
                              getprogname());
    exit(EXIT_SUCCESS);
}

static
int
parse_rflag(const char *arg)
{
    if (strlen(arg) != 1 || !isdigit(arg[0]))
        usage_error("Invalid value for -r; must be a single digit");

    return arg[0] - '0';
}

int
main(int argc, char **argv)
{
    bool lflag;
    int ch;
    struct atf_tp tp;

    atf_tp_init(&tp);

    lflag = false;
    while ((ch = getopt(argc, argv, ":hlr:s:v:")) != -1) {
        switch (ch) {
        case 'h':
            usage();
            break;

        case 'l':
            lflag = true;
            break;

        case 'r':
            atf_tp_set_results_fd(&tp, parse_rflag(optarg));
            break;

        case 's':
            break;

        case 'v':
            usage_error("foo");
            break;

        case ':':
            break;

        case '?':
        default:
            usage_error("bar");
        }
    }
    argc -= optind;
    argv += optind;

    atf_tp_add_tcs(&tp);
    return atf_tp_run(&tp);
}
