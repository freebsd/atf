//
// Automated Testing Framework (atf)
//
// Copyright (c) 2008 The NetBSD Foundation, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this
//    software must display the following acknowledgement:
//        This product includes software developed by the NetBSD
//        Foundation, Inc. and its contributors.
// 4. Neither the name of The NetBSD Foundation nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
// CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <stdio.h>
#include <string.h>

#include <atf.h>

#include "atf-c/fs.h"

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
        atf_error_t err;

        printf("Input          : >%s<\n", t->in);
        printf("Expected output: >%s<\n", t->out);

        err = atf_fs_path_init_fmt(&p, "%s", t->in);
        ATF_CHECK(!atf_is_error(err));
        printf("Output         : >%s<\n", atf_fs_path_cstring(&p));
        ATF_CHECK(strcmp(atf_fs_path_cstring(&p), t->out) == 0);
        atf_fs_path_fini(&p);

        printf("\n");
    }
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
        atf_error_t err;

        printf("Input          : %s\n", t->in);
        printf("Expected result: %s\n", t->abs ? "true" : "false");

        err = atf_fs_path_init_fmt(&p, "%s", t->in);
        ATF_CHECK(!atf_is_error(err));
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
        atf_error_t err;

        printf("Input          : %s\n", t->in);
        printf("Expected result: %s\n", t->root ? "true" : "false");

        err = atf_fs_path_init_fmt(&p, "%s", t->in);
        ATF_CHECK(!atf_is_error(err));
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
        atf_error_t err;

        printf("Input          : %s\n", t->in);
        printf("Expected output: %s\n", t->branch);

        err = atf_fs_path_init_fmt(&p, "%s", t->in);
        ATF_CHECK(!atf_is_error(err));

        err = atf_fs_path_branch_path(&p, &bp);
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
        atf_error_t err;

        printf("Input          : %s\n", t->in);
        printf("Expected output: %s\n", t->leaf);

        err = atf_fs_path_init_fmt(&p, "%s", t->in);
        ATF_CHECK(!atf_is_error(err));

        err = atf_fs_path_leaf_name(&p, &ln);
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
        atf_error_t err;

        printf("Input          : >%s<\n", t->in);
        printf("Append         : >%s<\n", t->ap);
        printf("Expected output: >%s<\n", t->out);

        err = atf_fs_path_init_fmt(&p, "%s", t->in);
        ATF_CHECK(!atf_is_error(err));

        err = atf_fs_path_append_fmt(&p, "%s", t->ap);
        ATF_CHECK(!atf_is_error(err));

        printf("Output         : >%s<\n", atf_fs_path_cstring(&p));
        ATF_CHECK(strcmp(atf_fs_path_cstring(&p), t->out) == 0);

        atf_fs_path_fini(&p);

        printf("\n");
    }
}

ATF_TC(path_to_absolute);
ATF_TC_HEAD(path_to_absolute, tc)
{
    atf_tc_set_var("descr", "Tests the conversion of a relative path to an absolute "
                 "one");
}
ATF_TC_BODY(path_to_absolute, tc)
{
/*

    create_files();

    {
        const path p(".");
        path pa = p.to_absolute();
        ATF_CHECK(pa.is_absolute());

        file_info fi(p);
        file_info fia(pa);
        ATF_CHECK_EQUAL(fi.get_device(), fia.get_device());
        ATF_CHECK_EQUAL(fi.get_inode(), fia.get_inode());
    }

    {
        const path p("files/reg");
        path pa = p.to_absolute();
        ATF_CHECK(pa.is_absolute());

        file_info fi(p);
        file_info fia(pa);
        ATF_CHECK_EQUAL(fi.get_device(), fia.get_device());
        ATF_CHECK_EQUAL(fi.get_inode(), fia.get_inode());
    }
*/
}

/*
ATF_TC(path_op_less);
ATF_TC_HEAD(path_op_less)
{
    atf_tc_set_var("descr", "Tests that the path's less-than operator works");
}
ATF_TC_BODY(path_op_less)
{
    create_files();

    ATF_CHECK(!(path("aaa") < path("aaa")));

    ATF_CHECK(  path("aab") < path("abc"));
    ATF_CHECK(!(path("abc") < path("aab")));
}
*/

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    /* Add the tests for the "atf_fs_path" type. */
    ATF_TP_ADD_TC(tp, path_normalize);
    ATF_TP_ADD_TC(tp, path_is_absolute);
    ATF_TP_ADD_TC(tp, path_is_root);
    ATF_TP_ADD_TC(tp, path_branch_path);
    ATF_TP_ADD_TC(tp, path_leaf_name);
    /*
    ATF_TP_ADD_TC(tp, path_compare_equal);
    ATF_TP_ADD_TC(tp, path_compare_different);
    */
    ATF_TP_ADD_TC(tp, path_append);
    ATF_TP_ADD_TC(tp, path_to_absolute);
    /*
    ATF_TP_ADD_TC(tp, path_op_less);
    */

    return 0;
}
