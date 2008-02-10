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

// XXX Must not use C++ here.

#include <iostream>

extern "C" {
#include "atf-c/dynstr.h"
}

#include "atf/macros.hpp"

ATF_TEST_CASE(init);
ATF_TEST_CASE_HEAD(init)
{
    set("descr", "XXX");
}
ATF_TEST_CASE_BODY(init)
{
    struct atf_dynstr ad;

    atf_dynstr_init(&ad);
    atf_dynstr_fini(&ad);
}

ATF_TEST_CASE(init_ap);
ATF_TEST_CASE_HEAD(init_ap)
{
    set("descr", "XXX");
}
ATF_TEST_CASE_BODY(init_ap)
{
    struct atf_dynstr ad;

    atf_dynstr_init(&ad);
    atf_dynstr_fini(&ad);
}

ATF_TEST_CASE(init_fmt);
ATF_TEST_CASE_HEAD(init_fmt)
{
    set("descr", "XXX");
}
ATF_TEST_CASE_BODY(init_fmt)
{
    const char *cstr;
    struct atf_dynstr ad;

    atf_dynstr_init_fmt(&ad, "A string");
    cstr = atf_dynstr_cstring(&ad);
    ATF_CHECK(strcmp(cstr, "A string") == 0);
    atf_dynstr_fini(&ad);

    atf_dynstr_init_fmt(&ad, "A string, an int %d", 5);
    cstr = atf_dynstr_cstring(&ad);
    ATF_CHECK(strcmp(cstr, "A string, an int 5") == 0);
    atf_dynstr_fini(&ad);

    atf_dynstr_init_fmt(&ad, "A %s, an int %d", "string", 5);
    cstr = atf_dynstr_cstring(&ad);
    ATF_CHECK(strcmp(cstr, "A string, an int 5") == 0);
    atf_dynstr_fini(&ad);
}

ATF_TEST_CASE(init_rep);
ATF_TEST_CASE_HEAD(init_rep)
{
    set("descr", "XXX");
}
ATF_TEST_CASE_BODY(init_rep)
{
    const size_t maxlen = 1024;
    char buf[maxlen];
    struct atf_dynstr ad;

    buf[0] = '\0';

    for (size_t i = 0; i < maxlen; i++) {
        atf_dynstr_init_rep(&ad, i, 'a');
        if (strcmp(atf_dynstr_cstring(&ad), buf) != 0) {
            std::cout << "Failed at iteration " << i << std::endl;
            ATF_FAIL("Failed to construct pad");
        }
        atf_dynstr_fini(&ad);

        strcat(buf, "a");
    }
}

ATF_TEST_CASE(append);
ATF_TEST_CASE_HEAD(append)
{
    set("descr", "XXX");
}
ATF_TEST_CASE_BODY(append)
{
    const size_t maxlen = 1024;
    char buf[maxlen];
    struct atf_dynstr ad;

    buf[0] = '\0';

    atf_dynstr_init(&ad);
    for (size_t i = 0; i < maxlen; i++) {
        if (strcmp(atf_dynstr_cstring(&ad), buf) != 0) {
            std::cout << "Failed at iteration " << i << std::endl;
            ATF_FAIL("Failed to append character");
        }

        atf_dynstr_append(&ad, "a");
        strcat(buf, "a");
    }
    atf_dynstr_fini(&ad);
}

ATF_INIT_TEST_CASES(tcs)
{
    ATF_ADD_TEST_CASE(tcs, init);
    ATF_ADD_TEST_CASE(tcs, init_ap);
    ATF_ADD_TEST_CASE(tcs, init_fmt);
    ATF_ADD_TEST_CASE(tcs, init_rep);
    ATF_ADD_TEST_CASE(tcs, append);
}
