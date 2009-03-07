/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2009 The NetBSD Foundation, Inc.
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

#include <stdio.h>
#include <string.h>

#include <atf-c.h>

#include "atf-c/build.h"
#include "atf-c/config.h"
#include "atf-c/env.h"

#include "h_macros.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

void __atf_config_reinit(void);

static
bool
equal_list_array(const atf_list_t *l, const char *const *a)
{
    bool equal;

    if (atf_list_size(l) == 0 && *a == NULL)
        equal = true;
    else if (atf_list_size(l) == 0 || *a == NULL)
        equal = false;
    else {
        atf_list_citer_t liter;

        equal = true;
        atf_list_for_each_c(liter, l) {
            if (*a == NULL ||
                strcmp((const char *)atf_list_citer_data(liter), *a) != 0) {
                equal = false;
                break;
            }
            a++;
        }
    }

    return equal;
}

static
void
print_list(const char *prefix, const atf_list_t *l)
{
    atf_list_citer_t iter;

    printf("%s:", prefix);
    atf_list_for_each_c(iter, l)
        printf(" '%s'", (const char *)atf_list_citer_data(iter));
    printf("\n");
}

static
void
print_array(const char *prefix, const char *const *a)
{
    printf("%s:", prefix);
    for (; *a != NULL; a++)
        printf(" '%s'", *a);
    printf("\n");
}

static
void
check_equal_list_array(const atf_list_t *l, const char *const *a)
{
    print_array("Expected arguments", a);
    print_list("Arguments returned", l);

    if (!equal_list_array(l, a))
        atf_tc_fail_nonfatal("The constructed argv differs from the "
                             "expected values");
}

static
void
verbose_set_env(const char *var, const char *val)
{
    printf("Setting %s to '%s'\n", var, val);
    RE(atf_env_set(var, val));
}

/* ---------------------------------------------------------------------
 * Internal test cases.
 * --------------------------------------------------------------------- */

ATF_TC(equal_list_array);
ATF_TC_HEAD(equal_list_array, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the test case internal "
                      "equal_list_array function");
}
ATF_TC_BODY(equal_list_array, tc)
{
    {
        atf_list_t list;
        const char *const array[] = { NULL };

        RE(atf_list_init(&list));
        ATF_CHECK(equal_list_array(&list, array));
        atf_list_fini(&list);
    }

    {
        atf_list_t list;
        const char *const array[] = { NULL };

        RE(atf_list_init(&list));
        atf_list_append(&list, strdup("foo"), true);
        ATF_CHECK(!equal_list_array(&list, array));
        atf_list_fini(&list);
    }

    {
        atf_list_t list;
        const char *const array[] = { "foo", NULL };

        RE(atf_list_init(&list));
        ATF_CHECK(!equal_list_array(&list, array));
        atf_list_fini(&list);
    }

    {
        atf_list_t list;
        const char *const array[] = { "foo", NULL };

        RE(atf_list_init(&list));
        atf_list_append(&list, strdup("foo"), true);
        ATF_CHECK(equal_list_array(&list, array));
        atf_list_fini(&list);
    }
}

/* ---------------------------------------------------------------------
 * Test cases for the free functions.
 * --------------------------------------------------------------------- */

static struct c_o_test {
    const char *msg;
    const char *cc;
    const char *cflags;
    const char *cppflags;
    const char *sfile;
    const char *ofile;
    bool hasoptargs;
    const char *const optargs[16];
    const char *const expargv[16];
} c_o_tests[] = {
    {
        "No flags",
        "cc",
        "",
        "",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "cc", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Multi-word program name",
        "cc -foo",
        "",
        "",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "cc", "-foo", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Some cflags",
        "cc",
        "-f1 -f2    -f3 -f4-f5",
        "",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "cc", "-f1", "-f2", "-f3", "-f4-f5", "-o", "test.o",
            "-c", "test.c", NULL
        },
    },

    {
        "Some cppflags",
        "cc",
        "",
        "-f1 -f2    -f3 -f4-f5",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "cc", "-f1", "-f2", "-f3", "-f4-f5", "-o", "test.o",
            "-c", "test.c", NULL
        },
    },

    {
        "Some cflags and cppflags",
        "cc",
        "-f2",
        "-f1",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "cc", "-f1", "-f2", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Some optional arguments",
        "cc",
        "",
        "",
        "test.c",
        "test.o",
        true,
        {
            "-o1", "-o2", NULL
        },
        {
            "cc", "-o1", "-o2", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Some cflags, cppflags and optional arguments",
        "cc",
        "-f2",
        "-f1",
        "test.c",
        "test.o",
        true,
        {
            "-o1", "-o2", NULL
        },
        {
            "cc", "-f1", "-f2", "-o1", "-o2", "-o", "test.o",
            "-c", "test.c", NULL
        },
    },

    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        false,
        { NULL },
        { NULL },
    },
};

ATF_TC(c_o);
ATF_TC_HEAD(c_o, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the atf_build_c_o function");
}
ATF_TC_BODY(c_o, tc)
{
    struct c_o_test *test;

    for (test = c_o_tests; test->expargv[0] != NULL; test++) {
        printf("> Test: %s\n", test->msg);

        verbose_set_env("ATF_BUILD_CC", test->cc);
        verbose_set_env("ATF_BUILD_CFLAGS", test->cflags);
        verbose_set_env("ATF_BUILD_CPPFLAGS", test->cppflags);
        __atf_config_reinit();

        {
            atf_list_t argv;
            if (test->hasoptargs)
                RE(atf_build_c_o(test->sfile, test->ofile, test->optargs,
                                 &argv));
            else
                RE(atf_build_c_o(test->sfile, test->ofile, NULL, &argv));
            check_equal_list_array(&argv, test->expargv);
            atf_list_fini(&argv);
        }
    }
}

static struct cpp_test {
    const char *msg;
    const char *cpp;
    const char *cppflags;
    const char *sfile;
    const char *ofile;
    bool hasoptargs;
    const char *const optargs[16];
    const char *const expargv[16];
} cpp_tests[] = {
    {
        "No flags",
        "cpp",
        "",
        "test.c",
        "test.out",
        false,
        {
            NULL
        },
        {
            "cpp", "-o", "test.out", "test.c", NULL
        },
    },

    {
        "Multi-word program name",
        "cpp -foo",
        "",
        "test.c",
        "test.out",
        false,
        {
            NULL
        },
        {
            "cpp", "-foo", "-o", "test.out", "test.c", NULL
        },
    },

    {
        "Some cppflags",
        "cpp",
        "-f1 -f2    -f3 -f4-f5",
        "test.c",
        "test.out",
        false,
        {
            NULL
        },
        {
            "cpp", "-f1", "-f2", "-f3", "-f4-f5", "-o", "test.out",
            "test.c", NULL
        },
    },

    {
        "Some optional arguments",
        "cpp",
        "",
        "test.c",
        "test.out",
        true,
        {
            "-o1", "-o2", NULL
        },
        {
            "cpp", "-o1", "-o2", "-o", "test.out", "test.c", NULL
        },
    },

    {
        "Some cppflags and optional arguments",
        "cpp",
        "-f1",
        "test.c",
        "test.out",
        true,
        {
            "-o1", "-o2", NULL
        },
        {
            "cpp", "-f1", "-o1", "-o2", "-o", "test.out", "test.c", NULL
        },
    },

    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        false,
        { NULL },
        { NULL },
    },
};

ATF_TC(cpp);
ATF_TC_HEAD(cpp, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the atf_build_cpp function");
}
ATF_TC_BODY(cpp, tc)
{
    struct cpp_test *test;

    for (test = cpp_tests; test->expargv[0] != NULL; test++) {
        printf("> Test: %s\n", test->msg);

        verbose_set_env("ATF_BUILD_CPP", test->cpp);
        verbose_set_env("ATF_BUILD_CPPFLAGS", test->cppflags);
        __atf_config_reinit();

        {
            atf_list_t argv;
            if (test->hasoptargs)
                RE(atf_build_cpp(test->sfile, test->ofile, test->optargs,
                                 &argv));
            else
                RE(atf_build_cpp(test->sfile, test->ofile, NULL, &argv));
            check_equal_list_array(&argv, test->expargv);
            atf_list_fini(&argv);
        }
    }
}

static struct cxx_o_test {
    const char *msg;
    const char *cxx;
    const char *cxxflags;
    const char *cppflags;
    const char *sfile;
    const char *ofile;
    bool hasoptargs;
    const char *const optargs[16];
    const char *const expargv[16];
} cxx_o_tests[] = {
    {
        "No flags",
        "c++",
        "",
        "",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "c++", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Multi-word program name",
        "c++ -foo",
        "",
        "",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "c++", "-foo", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Some cxxflags",
        "c++",
        "-f1 -f2    -f3 -f4-f5",
        "",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "c++", "-f1", "-f2", "-f3", "-f4-f5", "-o", "test.o",
            "-c", "test.c", NULL
        },
    },

    {
        "Some cppflags",
        "c++",
        "",
        "-f1 -f2    -f3 -f4-f5",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "c++", "-f1", "-f2", "-f3", "-f4-f5", "-o", "test.o",
            "-c", "test.c", NULL
        },
    },

    {
        "Some cxxflags and cppflags",
        "c++",
        "-f2",
        "-f1",
        "test.c",
        "test.o",
        false,
        {
            NULL
        },
        {
            "c++", "-f1", "-f2", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Some optional arguments",
        "c++",
        "",
        "",
        "test.c",
        "test.o",
        true,
        {
            "-o1", "-o2", NULL
        },
        {
            "c++", "-o1", "-o2", "-o", "test.o", "-c", "test.c", NULL
        },
    },

    {
        "Some cxxflags, cppflags and optional arguments",
        "c++",
        "-f2",
        "-f1",
        "test.c",
        "test.o",
        true,
        {
            "-o1", "-o2", NULL
        },
        {
            "c++", "-f1", "-f2", "-o1", "-o2", "-o", "test.o",
            "-c", "test.c", NULL
        },
    },

    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        false,
        { NULL },
        { NULL },
    },
};

ATF_TC(cxx_o);
ATF_TC_HEAD(cxx_o, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the atf_build_cxx_o function");
}
ATF_TC_BODY(cxx_o, tc)
{
    struct cxx_o_test *test;

    for (test = cxx_o_tests; test->expargv[0] != NULL; test++) {
        printf("> Test: %s\n", test->msg);

        verbose_set_env("ATF_BUILD_CXX", test->cxx);
        verbose_set_env("ATF_BUILD_CXXFLAGS", test->cxxflags);
        verbose_set_env("ATF_BUILD_CPPFLAGS", test->cppflags);
        __atf_config_reinit();

        {
            atf_list_t argv;
            if (test->hasoptargs)
                RE(atf_build_cxx_o(test->sfile, test->ofile, test->optargs,
                                   &argv));
            else
                RE(atf_build_cxx_o(test->sfile, test->ofile, NULL, &argv));
            check_equal_list_array(&argv, test->expargv);
            atf_list_fini(&argv);
        }
    }
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    // Add the internal test cases.
    ATF_TP_ADD_TC(tp, equal_list_array);

    // Add the test cases for the free functions.
    ATF_TP_ADD_TC(tp, c_o);
    ATF_TP_ADD_TC(tp, cpp);
    ATF_TP_ADD_TC(tp, cxx_o);

    return atf_no_error();
}
