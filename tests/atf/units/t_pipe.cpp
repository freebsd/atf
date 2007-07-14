//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007 The NetBSD Foundation, Inc.
// All rights reserved.
//
// This code is derived from software contributed to The NetBSD Foundation
// by Julio M. Merino Vidal, developed as part of Google's Summer of Code
// 2007 program.
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

extern "C" {
#include <unistd.h>
}

#include <cstdlib>
#include <iostream>

#include <atf.hpp>

#include "atfprivate/pipe.hpp"
#include "atfprivate/systembuf.hpp"

ATF_TEST_CASE(tc_read_and_write);
ATF_TEST_CASE_HEAD(tc_read_and_write)
{
    set("descr", "Tests reading from and writing to a pipe");
}
ATF_TEST_CASE_BODY(tc_read_and_write)
{
    atf::pipe p;
    atf::systembuf rbuf(p.rend().get());
    atf::systembuf wbuf(p.wend().get());
    std::istream rend(&rbuf);
    std::ostream wend(&wbuf);

    // XXX This assumes that the pipe's buffer is big enough to accept
    // the data written without blocking!
    wend << "1Test 1message" << std::endl;
    std::string tmp;
    rend >> tmp;
    ATF_CHECK_EQUAL(tmp, "1Test");
    rend >> tmp;
    ATF_CHECK_EQUAL(tmp, "1message");
}

ATF_TEST_CASE(tc_remap_read);
ATF_TEST_CASE_HEAD(tc_remap_read)
{
    set("descr", "Tests the pipe::remap_read method");
}
ATF_TEST_CASE_BODY(tc_remap_read)
{
    atf::pipe p;
    atf::systembuf wbuf(p.wend().get());
    std::ostream wend(&wbuf);
    p.rend().posix_remap(STDIN_FILENO);

    // XXX This assumes that the pipe's buffer is big enough to accept
    // the data written without blocking!
    wend << "2Test 2message" << std::endl;
    std::string tmp;
    std::cin >> tmp;
    ATF_CHECK_EQUAL(tmp, "2Test");
    std::cin >> tmp;
    ATF_CHECK_EQUAL(tmp, "2message");
}

ATF_TEST_CASE(tc_remap_write);
ATF_TEST_CASE_HEAD(tc_remap_write)
{
    set("descr", "Tests the pipe::remap_write method");
}
ATF_TEST_CASE_BODY(tc_remap_write)
{
    atf::pipe p;
    atf::systembuf rbuf(p.rend().get());
    std::istream rend(&rbuf);
    p.wend().posix_remap(STDERR_FILENO);

    // XXX This assumes that the pipe's buffer is big enough to accept
    // the data written without blocking!
    std::cerr << "3Test 3message" << std::endl;
    std::string tmp;
    rend >> tmp;
    ATF_CHECK_EQUAL(tmp, "3Test");
    rend >> tmp;
    ATF_CHECK_EQUAL(tmp, "3message");
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&tc_read_and_write);
    tcs.push_back(&tc_remap_read);
    tcs.push_back(&tc_remap_write);
}
