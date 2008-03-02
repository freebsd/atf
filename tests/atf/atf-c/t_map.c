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

#include <stdio.h>
#include <string.h>

#include <atf-c.h>

#include "atf-c/map.h"

#define CE(stm) ATF_CHECK(!atf_is_error(stm))

/* ---------------------------------------------------------------------
 * Tests for the "atf_map" type.
 * --------------------------------------------------------------------- */

/*
 * Constructors and destructors.
 */

ATF_TC(map_init);
ATF_TC_HEAD(map_init, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_init function");
}
ATF_TC_BODY(map_init, tc)
{
    atf_map_t map;

    CE(atf_map_init(&map));
    ATF_CHECK_EQUAL(atf_map_size(&map), 0);
    atf_map_fini(&map);
}

/*
 * Getters.
 */

ATF_TC(find_c);
ATF_TC_HEAD(find_c, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_find_c function");
}
ATF_TC_BODY(find_c, tc)
{
    atf_map_t map;
    char val1[] = "V1";
    char val2[] = "V2";
    atf_map_citer_t iter;

    CE(atf_map_init(&map));
    CE(atf_map_insert(&map, "K1", val1, false));
    CE(atf_map_insert(&map, "K2", val2, false));

    iter = atf_map_find_c(&map, "K0");
    ATF_CHECK(atf_equal_map_citer_map_citer(iter, atf_map_end_c(&map)));

    iter = atf_map_find_c(&map, "K1");
    ATF_CHECK(!atf_equal_map_citer_map_citer(iter, atf_map_end_c(&map)));
    ATF_CHECK(strcmp(atf_map_citer_data(iter), "V1") == 0);

    iter = atf_map_find_c(&map, "K2");
    ATF_CHECK(!atf_equal_map_citer_map_citer(iter, atf_map_end_c(&map)));
    ATF_CHECK(strcmp(atf_map_citer_data(iter), "V2") == 0);

    atf_map_fini(&map);
}

ATF_TC(get_bool);
ATF_TC_HEAD(get_bool, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_get_bool function");
}
ATF_TC_BODY(get_bool, tc)
{
    atf_map_t map;
    bool b;

    CE(atf_map_init(&map));
    CE(atf_map_insert(&map, "T1", strdup("true"), true));
    CE(atf_map_insert(&map, "T2", strdup("TRUE"), true));
    CE(atf_map_insert(&map, "T3", strdup("yes"), true));
    CE(atf_map_insert(&map, "T4", strdup("YES"), true));
    CE(atf_map_insert(&map, "F1", strdup("false"), true));
    CE(atf_map_insert(&map, "F2", strdup("FALSE"), true));
    CE(atf_map_insert(&map, "F3", strdup("no"), true));
    CE(atf_map_insert(&map, "F4", strdup("NO"), true));
    CE(atf_map_insert(&map, "I1", strdup("true2"), true));
    CE(atf_map_insert(&map, "I2", strdup("false2"), true));
    CE(atf_map_insert(&map, "I3", strdup(""), true));
    CE(atf_map_insert(&map, "I4", strdup("foo"), true));

    CE(atf_map_get_bool(&map, "T1", &b)); ATF_CHECK(b);
    CE(atf_map_get_bool(&map, "T2", &b)); ATF_CHECK(b);
    CE(atf_map_get_bool(&map, "T3", &b)); ATF_CHECK(b);
    CE(atf_map_get_bool(&map, "T4", &b)); ATF_CHECK(b);

    CE(atf_map_get_bool(&map, "F1", &b)); ATF_CHECK(!b);
    CE(atf_map_get_bool(&map, "F2", &b)); ATF_CHECK(!b);
    CE(atf_map_get_bool(&map, "F3", &b)); ATF_CHECK(!b);
    CE(atf_map_get_bool(&map, "F4", &b)); ATF_CHECK(!b);

    ATF_CHECK(atf_is_error(atf_map_get_bool(&map, "I1", &b)));
    ATF_CHECK(atf_is_error(atf_map_get_bool(&map, "I2", &b)));
    ATF_CHECK(atf_is_error(atf_map_get_bool(&map, "I3", &b)));
    ATF_CHECK(atf_is_error(atf_map_get_bool(&map, "I4", &b)));

    atf_map_fini(&map);
}

ATF_TC(get_bool_wd);
ATF_TC_HEAD(get_bool_wd, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_get_bool_wd function");
}
ATF_TC_BODY(get_bool_wd, tc)
{
    atf_map_t map;
    bool b;

    CE(atf_map_init(&map));
    CE(atf_map_insert(&map, "T1", strdup("true"), true));
    CE(atf_map_insert(&map, "F1", strdup("false"), true));
    CE(atf_map_insert(&map, "I1", strdup("true2"), true));

    CE(atf_map_get_bool_wd(&map, "T1", false, &b)); ATF_CHECK(b);
    CE(atf_map_get_bool_wd(&map, "F1", true, &b)); ATF_CHECK(!b);
    ATF_CHECK(atf_is_error(atf_map_get_bool_wd(&map, "I1", false, &b)));
    CE(atf_map_get_bool_wd(&map, "U1", true, &b)); ATF_CHECK(b);
    CE(atf_map_get_bool_wd(&map, "U2", false, &b)); ATF_CHECK(!b);

    atf_map_fini(&map);
}

ATF_TC(get_cstring);
ATF_TC_HEAD(get_cstring, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_get_cstring function");
}
ATF_TC_BODY(get_cstring, tc)
{
    atf_map_t map;
    const char *str;

    CE(atf_map_init(&map));
    CE(atf_map_insert(&map, "1", strdup("foo"), true));
    CE(atf_map_insert(&map, "2", strdup("bar"), true));

    CE(atf_map_get_cstring(&map, "1", &str));
    ATF_CHECK(strcmp(str, "foo") == 0);
    CE(atf_map_get_cstring(&map, "2", &str));
    ATF_CHECK(strcmp(str, "bar") == 0);

    atf_map_fini(&map);
}

ATF_TC(get_cstring_wd);
ATF_TC_HEAD(get_cstring_wd, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_get_cstring_wd function");
}
ATF_TC_BODY(get_cstring_wd, tc)
{
    atf_map_t map;
    const char *str;

    CE(atf_map_init(&map));
    CE(atf_map_insert(&map, "1", strdup("foo"), true));
    CE(atf_map_insert(&map, "2", strdup("bar"), true));

    CE(atf_map_get_cstring_wd(&map, "1", "bar", &str));
    ATF_CHECK(strcmp(str, "foo") == 0);
    CE(atf_map_get_cstring_wd(&map, "2", "foo", &str));
    ATF_CHECK(strcmp(str, "bar") == 0);
    CE(atf_map_get_cstring_wd(&map, "3", "baz", &str));
    ATF_CHECK(strcmp(str, "baz") == 0);

    atf_map_fini(&map);
}

ATF_TC(get_long);
ATF_TC_HEAD(get_long, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_get_long function");
}
ATF_TC_BODY(get_long, tc)
{
    atf_map_t map;
    long l;

    CE(atf_map_init(&map));
    CE(atf_map_insert(&map, "O1", strdup("0"), true));
    CE(atf_map_insert(&map, "O2", strdup("-5"), true));
    CE(atf_map_insert(&map, "O3", strdup("5"), true));
    CE(atf_map_insert(&map, "O4", strdup("123456789"), true));
    CE(atf_map_insert(&map, "E1", strdup(""), true));
    CE(atf_map_insert(&map, "E2", strdup("foo"), true));
    CE(atf_map_insert(&map, "E3", strdup("12345x"), true));

    CE(atf_map_get_long(&map, "O1", &l)); ATF_CHECK_EQUAL(l, 0);
    CE(atf_map_get_long(&map, "O2", &l)); ATF_CHECK_EQUAL(l, -5);
    CE(atf_map_get_long(&map, "O3", &l)); ATF_CHECK_EQUAL(l, 5);
    CE(atf_map_get_long(&map, "O4", &l)); ATF_CHECK_EQUAL(l, 123456789);
    ATF_CHECK(atf_is_error(atf_map_get_long(&map, "E1", &l)));
    ATF_CHECK(atf_is_error(atf_map_get_long(&map, "E2", &l)));
    ATF_CHECK(atf_is_error(atf_map_get_long(&map, "E3", &l)));

    atf_map_fini(&map);
}

ATF_TC(get_long_wd);
ATF_TC_HEAD(get_long_wd, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_get_long_wd function");
}
ATF_TC_BODY(get_long_wd, tc)
{
    atf_map_t map;
    long l;

    CE(atf_map_init(&map));
    CE(atf_map_insert(&map, "O1", strdup("0"), true));
    CE(atf_map_insert(&map, "O2", strdup("-5"), true));
    CE(atf_map_insert(&map, "O3", strdup("5"), true));
    CE(atf_map_insert(&map, "E1", strdup(""), true));

    CE(atf_map_get_long_wd(&map, "O1", 123, &l)); ATF_CHECK_EQUAL(l, 0);
    CE(atf_map_get_long_wd(&map, "O2", 123, &l)); ATF_CHECK_EQUAL(l, -5);
    CE(atf_map_get_long_wd(&map, "O3", 123, &l)); ATF_CHECK_EQUAL(l, 5);
    ATF_CHECK(atf_is_error(atf_map_get_long_wd(&map, "E1", 123, &l)));
    CE(atf_map_get_long_wd(&map, "U1", 123, &l)); ATF_CHECK_EQUAL(l, 123);

    atf_map_fini(&map);
}

/*
 * Modifiers.
 */

ATF_TC(map_insert);
ATF_TC_HEAD(map_insert, tc)
{
    atf_tc_set_var(tc, "descr", "Checks the atf_map_insert function");
}
ATF_TC_BODY(map_insert, tc)
{
    atf_map_t map;
    char buf[] = "1st test string";
    char buf2[] = "2nd test string";
    const char *ptr;

    CE(atf_map_init(&map));

    printf("Inserting some values\n");
    ATF_CHECK_EQUAL(atf_map_size(&map), 0);
    CE(atf_map_insert(&map, "K1", buf, false));
    ATF_CHECK_EQUAL(atf_map_size(&map), 1);
    CE(atf_map_insert(&map, "K2", buf, false));
    ATF_CHECK_EQUAL(atf_map_size(&map), 2);
    CE(atf_map_insert(&map, "K3", buf, false));
    ATF_CHECK_EQUAL(atf_map_size(&map), 3);

    printf("Replacing a value\n");
    CE(atf_map_get_cstring(&map, "K3", &ptr)); ATF_CHECK_EQUAL(ptr, buf);
    CE(atf_map_insert(&map, "K3", buf2, false));
    ATF_CHECK_EQUAL(atf_map_size(&map), 3);
    CE(atf_map_get_cstring(&map, "K3", &ptr)); ATF_CHECK_EQUAL(ptr, buf2);

    atf_map_fini(&map);
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    /* Constructors and destructors. */
    ATF_TP_ADD_TC(tp, map_init);

    /* Getters. */
    ATF_TP_ADD_TC(tp, find_c);
    ATF_TP_ADD_TC(tp, get_bool);
    ATF_TP_ADD_TC(tp, get_bool_wd);
    ATF_TP_ADD_TC(tp, get_cstring);
    ATF_TP_ADD_TC(tp, get_cstring_wd);
    ATF_TP_ADD_TC(tp, get_long);
    ATF_TP_ADD_TC(tp, get_long_wd);

    /* Modifiers. */
    ATF_TP_ADD_TC(tp, map_insert);

    return atf_no_error();
}
