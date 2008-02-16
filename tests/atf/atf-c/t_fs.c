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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <atf.h>

#include "atf-c/fs.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

#define CE(stm) ATF_CHECK(!atf_is_error(stm))

static
void
create_dir(const char *p, int mode)
{
    int fd;

    fd = mkdir(p, mode);
    if (fd == -1)
        atf_tc_fail("Could not create helper directory %s", p);
}

static
void
create_file(const char *p, int mode)
{
    int fd;

    fd = open(p, O_CREAT | O_WRONLY | O_TRUNC, mode);
    if (fd == -1)
        atf_tc_fail("Could not create helper file %s", p);
}

/* ---------------------------------------------------------------------
 * Test cases for the "atf_fs_path" type.
 * --------------------------------------------------------------------- */

ATF_TC(path_normalize);
ATF_TC_HEAD(path_normalize, tc)
{
    atf_tc_set_var("descr", "Tests the path's normalization");
}
ATF_TC_BODY(path_normalize, tc)
{
    struct test {
        const char *in;
        const char *out;
    } tests[] = {
        { ".", ".", },
        { "..", "..", },

        { "/", "/", },
        { "//", "/", },
        { "///", "/", },

        { "foo", "foo", },
        { "foo/", "foo", },
        { "foo/bar", "foo/bar", },
        { "foo/bar/", "foo/bar", },

        { "/foo", "/foo", },
        { "/foo/bar", "/foo/bar", },
        { "/foo/bar/", "/foo/bar", },

        { "///foo", "/foo", },
        { "///foo///bar", "/foo/bar", },
        { "///foo///bar///", "/foo/bar", },

        { NULL, NULL }
    };
    struct test *t;

    for (t = &tests[0]; t->in != NULL; t++) {
        atf_fs_path_t p;

        printf("Input          : >%s<\n", t->in);
        printf("Expected output: >%s<\n", t->out);

        CE(atf_fs_path_init_fmt(&p, "%s", t->in));
        printf("Output         : >%s<\n", atf_fs_path_cstring(&p));
        ATF_CHECK(strcmp(atf_fs_path_cstring(&p), t->out) == 0);
        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_copy);
ATF_TC_HEAD(path_copy, tc)
{
    atf_tc_set_var("descr", "Tests the atf_fs_path_copy constructor");
}
ATF_TC_BODY(path_copy, tc)
{
    atf_fs_path_t str, str2;

    CE(atf_fs_path_init_fmt(&str, "foo"));
    CE(atf_fs_path_copy(&str2, &str));

    ATF_CHECK(atf_equal_fs_path_fs_path(&str, &str2));

    CE(atf_fs_path_append_fmt(&str2, "bar"));

    ATF_CHECK(!atf_equal_fs_path_fs_path(&str, &str2));

    atf_fs_path_fini(&str2);
    atf_fs_path_fini(&str);
}

ATF_TC(path_is_absolute);
ATF_TC_HEAD(path_is_absolute, tc)
{
    atf_tc_set_var("descr", "Tests the path::is_absolute function");
}
ATF_TC_BODY(path_is_absolute, tc)
{
    struct test {
        const char *in;
        bool abs;
    } tests[] = {
        { "/", true },
        { "////", true },
        { "////a", true },
        { "//a//", true },
        { "a////", false },
        { "../foo", false },
        { NULL, false },
    };
    struct test *t;

    for (t = &tests[0]; t->in != NULL; t++) {
        atf_fs_path_t p;

        printf("Input          : %s\n", t->in);
        printf("Expected result: %s\n", t->abs ? "true" : "false");

        CE(atf_fs_path_init_fmt(&p, "%s", t->in));
        printf("Result         : %s\n",
               atf_fs_path_is_absolute(&p) ? "true" : "false");
        if (t->abs)
            ATF_CHECK(atf_fs_path_is_absolute(&p));
        else
            ATF_CHECK(!atf_fs_path_is_absolute(&p));
        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_is_root);
ATF_TC_HEAD(path_is_root, tc)
{
    atf_tc_set_var("descr", "Tests the path::is_root function");
}
ATF_TC_BODY(path_is_root, tc)
{
    struct test {
        const char *in;
        bool root;
    } tests[] = {
        { "/", true },
        { "////", true },
        { "////a", false },
        { "//a//", false },
        { "a////", false },
        { "../foo", false },
        { NULL, false },
    };
    struct test *t;

    for (t = &tests[0]; t->in != NULL; t++) {
        atf_fs_path_t p;

        printf("Input          : %s\n", t->in);
        printf("Expected result: %s\n", t->root ? "true" : "false");

        CE(atf_fs_path_init_fmt(&p, "%s", t->in));
        printf("Result         : %s\n",
               atf_fs_path_is_root(&p) ? "true" : "false");
        if (t->root)
            ATF_CHECK(atf_fs_path_is_root(&p));
        else
            ATF_CHECK(!atf_fs_path_is_root(&p));
        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_branch_path);
ATF_TC_HEAD(path_branch_path, tc)
{
    atf_tc_set_var("descr", "Tests the atf_fs_path_branch_path function");
}
ATF_TC_BODY(path_branch_path, tc)
{
    struct test {
        const char *in;
        const char *branch;
    } tests[] = {
        { ".", "." },
        { "foo", "." },
        { "foo/bar", "foo" },
        { "/foo", "/" },
        { "/foo/bar", "/foo" },
        { NULL, NULL },
    };
    struct test *t;

    for (t = &tests[0]; t->in != NULL; t++) {
        atf_fs_path_t p, bp;

        printf("Input          : %s\n", t->in);
        printf("Expected output: %s\n", t->branch);

        CE(atf_fs_path_init_fmt(&p, "%s", t->in));
        CE(atf_fs_path_branch_path(&p, &bp));
        printf("Output         : %s\n", atf_fs_path_cstring(&bp));
        ATF_CHECK(strcmp(atf_fs_path_cstring(&bp), t->branch) == 0);
        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_leaf_name);
ATF_TC_HEAD(path_leaf_name, tc)
{
    atf_tc_set_var("descr", "Tests the atf_fs_path_leaf_name function");
}
ATF_TC_BODY(path_leaf_name, tc)
{
    struct test {
        const char *in;
        const char *leaf;
    } tests[] = {
        { ".", "." },
        { "foo", "foo" },
        { "foo/bar", "bar" },
        { "/foo", "foo" },
        { "/foo/bar", "bar" },
        { NULL, NULL },
    };
    struct test *t;

    for (t = &tests[0]; t->in != NULL; t++) {
        atf_fs_path_t p;
        atf_dynstr_t ln;

        printf("Input          : %s\n", t->in);
        printf("Expected output: %s\n", t->leaf);

        CE(atf_fs_path_init_fmt(&p, "%s", t->in));
        CE(atf_fs_path_leaf_name(&p, &ln));
        printf("Output         : %s\n", atf_dynstr_cstring(&ln));
        ATF_CHECK(atf_equal_dynstr_cstring(&ln, t->leaf));
        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_append);
ATF_TC_HEAD(path_append, tc)
{
    atf_tc_set_var("descr", "Tests the concatenation of multiple paths");
}
ATF_TC_BODY(path_append, tc)
{
    struct test {
        const char *in;
        const char *ap;
        const char *out;
    } tests[] = {
        { "foo", "bar", "foo/bar" },
        { "foo/", "/bar", "foo/bar" },
        { "foo/", "/bar/baz", "foo/bar/baz" },
        { "foo/", "///bar///baz", "foo/bar/baz" },

        { NULL, NULL, NULL }
    };
    struct test *t;

    for (t = &tests[0]; t->in != NULL; t++) {
        atf_fs_path_t p;

        printf("Input          : >%s<\n", t->in);
        printf("Append         : >%s<\n", t->ap);
        printf("Expected output: >%s<\n", t->out);

        CE(atf_fs_path_init_fmt(&p, "%s", t->in));

        CE(atf_fs_path_append_fmt(&p, "%s", t->ap));

        printf("Output         : >%s<\n", atf_fs_path_cstring(&p));
        ATF_CHECK(strcmp(atf_fs_path_cstring(&p), t->out) == 0);

        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_to_absolute);
ATF_TC_HEAD(path_to_absolute, tc)
{
    atf_tc_set_var("descr", "Tests the atf_fs_path_to_absolute function");
}
ATF_TC_BODY(path_to_absolute, tc)
{
    const char *names[] = { ".", "dir", NULL };
    const char **n;

    ATF_CHECK(mkdir("dir", 0755) != -1);

    for (n = names; *n != NULL; n++) {
        atf_fs_path_t p;
        atf_fs_stat_t st1, st2;

        CE(atf_fs_path_init_fmt(&p, "%s", *n));
        CE(atf_fs_stat_init(&st1, &p));
        printf("Relative path: %s\n", atf_fs_path_cstring(&p));

        CE(atf_fs_path_to_absolute(&p));
        printf("Absolute path: %s\n", atf_fs_path_cstring(&p));

        ATF_CHECK(atf_fs_path_is_absolute(&p));
        CE(atf_fs_stat_init(&st2, &p));

        ATF_CHECK_EQUAL(atf_fs_stat_get_device(&st1),
                        atf_fs_stat_get_device(&st2));
        ATF_CHECK_EQUAL(atf_fs_stat_get_inode(&st1),
                        atf_fs_stat_get_inode(&st2));

        atf_fs_stat_fini(&st2);
        atf_fs_stat_fini(&st1);
        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_equal);
ATF_TC_HEAD(path_equal, tc)
{
    atf_tc_set_var("descr", "Tests the equality operators for paths");
}
ATF_TC_BODY(path_equal, tc)
{
    atf_fs_path_t p1, p2;

    CE(atf_fs_path_init_fmt(&p1, "foo"));

    CE(atf_fs_path_init_fmt(&p2, "foo"));
    ATF_CHECK(atf_equal_fs_path_fs_path(&p1, &p2));
    atf_fs_path_fini(&p2);

    CE(atf_fs_path_init_fmt(&p2, "bar"));
    ATF_CHECK(!atf_equal_fs_path_fs_path(&p1, &p2));
    atf_fs_path_fini(&p2);

    atf_fs_path_fini(&p1);
}

/* ---------------------------------------------------------------------
 * Test cases for the "atf_fs_stat" type.
 * --------------------------------------------------------------------- */

ATF_TC(stat_type);
ATF_TC_HEAD(stat_type, tc)
{
    atf_tc_set_var("descr", "Tests the atf_fs_stat_get_type function and, "
                            "indirectly, the constructor");
}
ATF_TC_BODY(stat_type, tc)
{
    atf_fs_path_t p;
    atf_fs_stat_t st;

    create_dir("dir", 0755);
    create_file("reg", 0644);

    CE(atf_fs_path_init_fmt(&p, "dir"));
    CE(atf_fs_stat_init(&st, &p));
    ATF_CHECK_EQUAL(atf_fs_stat_get_type(&st), atf_fs_stat_dir_type);
    atf_fs_stat_fini(&st);
    atf_fs_path_fini(&p);

    CE(atf_fs_path_init_fmt(&p, "reg"));
    CE(atf_fs_stat_init(&st, &p));
    ATF_CHECK_EQUAL(atf_fs_stat_get_type(&st), atf_fs_stat_reg_type);
    atf_fs_stat_fini(&st);
    atf_fs_path_fini(&p);
}

ATF_TC(stat_perms);
ATF_TC_HEAD(stat_perms, tc)
{
    atf_tc_set_var("descr", "Tests the atf_fs_stat_is_* functions");
}
ATF_TC_BODY(stat_perms, tc)
{
    atf_fs_path_t p;
    atf_fs_stat_t st;

    create_file("reg", 0);

    CE(atf_fs_path_init_fmt(&p, "reg"));

#define perms(ur, uw, ux, gr, gw, gx, othr, othw, othx) \
    { \
        CE(atf_fs_stat_init(&st, &p)); \
        ATF_CHECK(atf_fs_stat_is_owner_readable(&st) == ur); \
        ATF_CHECK(atf_fs_stat_is_owner_writable(&st) == uw); \
        ATF_CHECK(atf_fs_stat_is_owner_executable(&st) == ux); \
        ATF_CHECK(atf_fs_stat_is_group_readable(&st) == gr); \
        ATF_CHECK(atf_fs_stat_is_group_writable(&st) == gw); \
        ATF_CHECK(atf_fs_stat_is_group_executable(&st) == gx); \
        ATF_CHECK(atf_fs_stat_is_other_readable(&st) == othr); \
        ATF_CHECK(atf_fs_stat_is_other_writable(&st) == othw); \
        ATF_CHECK(atf_fs_stat_is_other_executable(&st) == othx); \
        atf_fs_stat_fini(&st); \
    }

    chmod("reg", 0000);
    perms(false, false, false, false, false, false, false, false, false);

    chmod("reg", 0001);
    perms(false, false, false, false, false, false, false, false, true);

    chmod("reg", 0010);
    perms(false, false, false, false, false, true, false, false, false);

    chmod("reg", 0100);
    perms(false, false, true, false, false, false, false, false, false);

    chmod("reg", 0002);
    perms(false, false, false, false, false, false, false, true, false);

    chmod("reg", 0020);
    perms(false, false, false, false, true, false, false, false, false);

    chmod("reg", 0200);
    perms(false, true, false, false, false, false, false, false, false);

    chmod("reg", 0004);
    perms(false, false, false, false, false, false, true, false, false);

    chmod("reg", 0040);
    perms(false, false, false, true, false, false, false, false, false);

    chmod("reg", 0400);
    perms(true, false, false, false, false, false, false, false, false);

    chmod("reg", 0644);
    perms(true, true, false, true, false, false, true, false, false);

    chmod("reg", 0755);
    perms(true, true, true, true, false, true, true, false, true);

    chmod("reg", 0777);
    perms(true, true, true, true, true, true, true, true, true);

#undef perms

    atf_fs_path_fini(&p);
}

/* ---------------------------------------------------------------------
 * Test cases for the free functions.
 * --------------------------------------------------------------------- */

ATF_TC(cleanup);
ATF_TC_HEAD(cleanup, tc)
{
    atf_tc_set_var("descr", "Tests the atf_fs_cleanup function");
}
ATF_TC_BODY(cleanup, tc)
{
    atf_fs_path_t root;

    create_dir ("root", 0755);
    create_dir ("root/dir", 0755);
    create_dir ("root/dir/1", 0755);
    create_file("root/dir/2", 0644);
    create_file("root/reg", 0644);

    CE(atf_fs_path_init_fmt(&root, "root"));
    CE(atf_fs_cleanup(&root));
    ATF_CHECK(access("root", F_OK) == -1 && errno == ENOENT);
    atf_fs_path_fini(&root);

    /* TODO: Cleanup with mount points, just as in tools/t_atf_cleanup. */
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    /* Add the tests for the "atf_fs_path" type. */
    ATF_TP_ADD_TC(tp, path_normalize);
    ATF_TP_ADD_TC(tp, path_copy);
    ATF_TP_ADD_TC(tp, path_is_absolute);
    ATF_TP_ADD_TC(tp, path_is_root);
    ATF_TP_ADD_TC(tp, path_branch_path);
    ATF_TP_ADD_TC(tp, path_leaf_name);
    ATF_TP_ADD_TC(tp, path_append);
    ATF_TP_ADD_TC(tp, path_to_absolute);
    ATF_TP_ADD_TC(tp, path_equal);

    /* Add the tests for the "atf_fs_stat" type. */
    ATF_TP_ADD_TC(tp, stat_type);
    ATF_TP_ADD_TC(tp, stat_perms);

    /* Add the tests for the free functions. */
    ATF_TP_ADD_TC(tp, cleanup);

    return 0;
}
