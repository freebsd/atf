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

#if defined(HAVE_CONFIG_H)
#include "bconfig.h"
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atf-c/dynstr.h"
#include "atf-c/expand.h"
#include "atf-c/object.h"
#include "atf-c/sanity.h"
#include "atf-c/tc.h"
#include "atf-c/tp.h"
#include "atf-c/ui.h"

#if defined(HAVE_GNU_GETOPT)
#   define GETOPT_POSIX "+"
#else
#   define GETOPT_POSIX ""
#endif

/* ---------------------------------------------------------------------
 * The "usage" and "user" error types.
 * --------------------------------------------------------------------- */

#define FREE_FORM_ERROR(name) \
    struct name ## _error_data { \
        char m_what[2048]; \
    }; \
    \
    static \
    void \
    name ## _format(const atf_error_t err, char *buf, size_t buflen) \
    { \
        const struct name ## _error_data *data; \
        \
        PRE(atf_error_is(err, #name)); \
        \
        data = atf_error_data(err); \
        snprintf(buf, buflen, "%s", data->m_what); \
    } \
    \
    static \
    atf_error_t \
    name ## _error(const char *fmt, ...) \
    { \
        atf_error_t err; \
        struct name ## _error_data data; \
        va_list ap; \
        \
        va_start(ap, fmt); \
        vsnprintf(data.m_what, sizeof(data.m_what), fmt, ap); \
        va_end(ap); \
        \
        err = atf_error_new(#name, &data, sizeof(data), name ## _format); \
        \
        return err; \
    }

FREE_FORM_ERROR(usage);
FREE_FORM_ERROR(user);

/* ---------------------------------------------------------------------
 * Printing functions.
 * --------------------------------------------------------------------- */

/* XXX: Why are these functions here?  We have got a ui module, and it
 * seems the correct place to put these.  Otherwise, the functions that
 * currently live there only format text, so they'd be moved to the text
 * module instead and kill ui completely. */

static
atf_error_t
print_tag(const char *tag, bool repeat, size_t col, const char *fmt, ...)
{
    atf_error_t err;
    va_list ap;
    atf_dynstr_t dest;

    err = atf_dynstr_init(&dest);
    if (atf_is_error(err))
        goto out;

    va_start(ap, fmt);
    err = atf_ui_format_ap(&dest, tag, repeat, col, fmt, ap);
    va_end(ap);
    if (atf_is_error(err))
        goto out_dest;

    printf("%s\n", atf_dynstr_cstring(&dest));

out_dest:
    atf_dynstr_fini(&dest);
out:
    return err;
}

static
void
print_error(const atf_error_t err)
{
    PRE(atf_is_error(err));

    if (atf_error_is(err, "no_memory")) {
        char buf[1024];

        atf_error_format(err, buf, sizeof(buf));

        fprintf(stderr, "%s: %s\n", getprogname(), buf);
    } else {
        atf_dynstr_t tag;
        char buf[4096];

        atf_error_format(err, buf, sizeof(buf));

        atf_dynstr_init_fmt(&tag, "%s: ", getprogname());
        print_tag(atf_dynstr_cstring(&tag), true, 0, "ERROR: %s", buf);

        if (atf_error_is(err, "usage"))
            print_tag(atf_dynstr_cstring(&tag), true, 0,
                      "Type `%s -h' for more details.", getprogname());

        atf_dynstr_fini(&tag);
    }
}

/* ---------------------------------------------------------------------
 * Options handling.
 * --------------------------------------------------------------------- */

struct params {
    bool m_do_list;
    bool m_do_usage;
    int m_fd;
    atf_list_t m_tcglobs;
};

static
void
params_fini(struct params *p)
{
    atf_list_iter_t iter;

    atf_list_for_each(iter, &p->m_tcglobs)
        free(atf_list_iter_data(iter));

    atf_list_fini(&p->m_tcglobs);
}

static
atf_error_t
parse_rflag(const char *arg, int *value)
{
    atf_error_t err;

    if (strlen(arg) != 1 || !isdigit(arg[0])) {
        err = usage_error("Invalid value for -r; must be a single digit.");
        goto out;
    }

    *value = arg[0] - '0';
    INV(*value >= 0 && *value <= 9);
    err = atf_no_error();

out:
    return err;
}

/* ---------------------------------------------------------------------
 * Test case filtering.
 * --------------------------------------------------------------------- */

static
atf_error_t
match_tcs(const atf_tp_t *tp, const char *glob, atf_list_t *ids)
{
    atf_error_t err;
    bool found;
    atf_list_citer_t iter;

    err = atf_no_error();
    found = false;
    atf_list_for_each_c(iter, atf_tp_get_tcs(tp)) {
        const atf_tc_t *tc = atf_list_citer_data(iter);
        const char *ident = atf_tc_get_ident(tc);

        if (atf_expand_is_glob(glob)) {
            bool matches;

            err = atf_expand_matches_glob(glob, ident, &matches);
            if (!atf_is_error(err) && matches) {
                err = atf_list_append(ids, strdup(ident));
                found = true;
            }
        } else {
            if (strcmp(glob, tc->m_ident) == 0) {
                err = atf_list_append(ids, strdup(ident));
                found = true;
            }
        }

        if (atf_is_error(err))
            break;
    }

    if (!atf_is_error(err) && !found)
        err = user_error("Unknown test case `%s'", glob);

    return err;
}

static
atf_error_t
filter_tcs(const atf_tp_t *tp, const atf_list_t *globs, atf_list_t *ids)
{
    atf_error_t err;
    atf_list_citer_t iter;

    err = atf_list_init(ids);
    if (atf_is_error(err))
        goto out;

    atf_list_for_each_c(iter, globs) {
        const char *glob = atf_list_citer_data(iter);
        err = match_tcs(tp, glob, ids);
        if (atf_is_error(err)) {
            atf_list_fini(ids);
            goto out;
        }
    }

out:
    return err;
}

static
atf_error_t
list_tcs(const atf_tp_t *tp, const atf_list_t *tcids)
{
    atf_error_t err;
    size_t col;
    atf_list_citer_t iter;

    PRE(atf_list_size(tcids) > 0);

    err = atf_no_error();

    /* Calculate column where to start descriptions. */
    col = 0;
    atf_list_for_each_c(iter, tcids) {
        const char *id = atf_list_citer_data(iter);
        const atf_tc_t *tc = atf_tp_get_tc(tp, id);
        const size_t len = strlen(atf_tc_get_ident(tc));

        if (col < len)
            col = len;
    }
    col += 4;

    /* Pretty-print test case identifiers and descriptions. */
    atf_list_for_each_c(iter, tcids) {
        const char *id = atf_list_citer_data(iter);
        const atf_tc_t *tc = atf_tp_get_tc(tp, id);
        const atf_dynstr_t *descr = atf_tc_get_var(tc, "descr");

        err = print_tag(id, false, col, "%s", atf_dynstr_cstring(descr));
        if (atf_is_error(err))
            break;
    }

    return err;
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

static
void
usage(void)
{
    print_tag("Usage: ", false, 0,
              "%s [options] [test_case1 [.. test_caseN]]",
              getprogname());
    printf("\n");
    print_tag("", false, 0, "This is an independent atf test program.");
    printf("\n");
    print_tag("", false, 0, "Available options:");
    print_tag("    -h              ", false, 0,
              "Shows this help message");
    print_tag("    -l              ", false, 0,
              "List test cases and their purpose");
    print_tag("    -r fd           ", false, 0,
              "The file descriptor to which the test program "
              "will send the results of the test cases");
    print_tag("    -s srcdir       ", false, 0,
              "Directory where the test's data files are "
              "located");
    print_tag("    -v var=value    ", false, 0,
              "Sets the configuration variable `var' to `value'");
    printf("\n");
    print_tag("", false, 0, "For more details please see "
              "atf-test-program(1) and atf(7).");
}

static
atf_error_t
process_params(int argc, char **argv, struct params *p)
{
    atf_error_t err;
    int ch;

    /* Initialize defaults. */
    p->m_do_list = false;
    p->m_do_usage = false;
    p->m_fd = STDOUT_FILENO;
    atf_list_init(&p->m_tcglobs);

    err = atf_no_error();
    while (!atf_is_error(err) &&
           (ch = getopt(argc, argv, GETOPT_POSIX ":hlr:s:v:")) != -1) {
        switch (ch) {
        case 'h':
            p->m_do_usage = true;
            break;

        case 'l':
            p->m_do_list = true;
            break;

        case 'r':
            err = parse_rflag(optarg, &p->m_fd);
            break;

        case 's':
            /* TODO */
            break;

        case 'v':
            /* TODO */
            break;

        case ':':
            err = usage_error("Option -%c requires an argument.", optopt);
            break;

        case '?':
        default:
            err = usage_error("Unknown option -%c.", optopt);
        }
    }
    argc -= optind;
    argv += optind;

    if (!atf_is_error(err)) {
        char **arg;
        for (arg = argv; !atf_is_error(err) && *arg != NULL; arg++)
            err = atf_list_append(&p->m_tcglobs, strdup(*arg));

        if (!atf_is_error(err) && atf_list_size(&p->m_tcglobs) == 0)
            err = atf_list_append(&p->m_tcglobs, strdup("*"));
    }

    if (atf_is_error(err))
        params_fini(p);

    return err;
}

static
atf_error_t
controlled_main(int argc, char **argv,
                atf_error_t (*add_tcs_hook)(atf_tp_t *),
                int *exitcode)
{
    atf_error_t err;
    struct params p;
    atf_tp_t tp;
    atf_list_t tcids;

    err = process_params(argc, argv, &p);
    if (atf_is_error(err))
        goto out;

    if (p.m_do_usage && argc != 2) {
        err = usage_error("-h must be given alone.");
        goto out_p;
    }

    if (p.m_do_usage) {
        usage();
        *exitcode = EXIT_SUCCESS;
        goto out_p;
    }

    atf_tp_init(&tp);

    err = add_tcs_hook(&tp);
    if (atf_is_error(err))
        goto out_tp;

    err = filter_tcs(&tp, &p.m_tcglobs, &tcids);
    if (atf_is_error(err))
        goto out_tp;

    if (p.m_do_list) {
        err = list_tcs(&tp, &tcids);
        if (!atf_is_error(err))
            *exitcode = EXIT_SUCCESS;
    } else {
        size_t failed;
        err = atf_tp_run(&tp, &tcids, p.m_fd, &failed);
        if (!atf_is_error(err))
            *exitcode = failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    {
        atf_list_iter_t iter;

        atf_list_for_each(iter, &tcids)
            free(atf_list_iter_data(iter));

        atf_list_fini(&tcids);
    }
out_tp:
    atf_tp_fini(&tp);
out_p:
    params_fini(&p);
out:
    return err;
}

int
atf_tp_main(int argc, char **argv, atf_error_t (*add_tcs_hook)(atf_tp_t *))
{
    atf_error_t err;
    int exitcode;

    atf_init_objects();

    exitcode = EXIT_FAILURE; /* Silence GCC warning. */
    err = controlled_main(argc, argv, add_tcs_hook, &exitcode);
    if (atf_is_error(err)) {
        print_error(err);
        atf_error_free(err);
        exitcode = EXIT_FAILURE;
    }

    return exitcode;
}
