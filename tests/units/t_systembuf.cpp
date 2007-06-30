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
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <unistd.h>
}

#include <cstddef>
#include <istream>
#include <fstream>
#include <ostream>

#include <atf.hpp>

#include "atfprivate/systembuf.hpp"

static
void
check_data(std::istream& is, std::size_t length)
{
    char ch = 'A', chr;
    std::size_t cnt = 0;
    while (is >> chr) {
        ATF_CHECK_EQUAL(ch, chr);
        if (ch == 'Z')
            ch = 'A';
        else
            ch++;
        cnt++;
    }
    ATF_CHECK_EQUAL(cnt, length);
}

static
void
write_data(std::ostream& os, std::size_t length)
{
    char ch = 'A';
    for (std::size_t i = 0; i < length; i++) {
        os << ch;
        if (ch == 'Z')
            ch = 'A';
        else
            ch++;
    }
    os.flush();
}

static
void
remove_file(const std::string& name)
{
    ::unlink(name.c_str());
}

static
void
test_read(std::size_t length, std::size_t bufsize)
{
    std::ofstream f("test_read.txt");
    write_data(f, length);
    f.close();

    int fd = ::open("test_read.txt", O_RDONLY);
    ATF_CHECK(fd != -1);
    atf::systembuf sb(fd, bufsize);
    std::istream is(&sb);
    check_data(is, length);
    ::close(fd);
    remove_file("test_read.txt");
}

static
void
test_write(std::size_t length, std::size_t bufsize)
{
    int fd = ::open("test_write.txt", O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    ATF_CHECK(fd != -1);
    atf::systembuf sb(fd, bufsize);
    std::ostream os(&sb);
    write_data(os, length);
    ::close(fd);

    std::ifstream is("test_write.txt");
    check_data(is, length);
    is.close();
    remove_file("test_write.txt");
}

ATF_TEST_CASE(tc_short_read);
ATF_TEST_CASE_HEAD(tc_short_read)
{
    set("descr", "Tests that a short read (one that fits in the "
                 "internal buffer) works");
}
ATF_TEST_CASE_BODY(tc_short_read)
{
    test_read(64, 1024);
    ATF_PASS();
}

ATF_TEST_CASE(tc_long_read);
ATF_TEST_CASE_HEAD(tc_long_read)
{
    set("descr", "Tests that a long read (one that does not fit in the "
                 "internal buffer) works");
}
ATF_TEST_CASE_BODY(tc_long_read)
{
    test_read(64 * 1024, 1024);
    ATF_PASS();
}

ATF_TEST_CASE(tc_short_write);
ATF_TEST_CASE_HEAD(tc_short_write)
{
    set("descr", "Tests that a short write (one that fits in the "
                 "internal buffer) works");
}
ATF_TEST_CASE_BODY(tc_short_write)
{
    test_write(64, 1024);
    ATF_PASS();
}

ATF_TEST_CASE(tc_long_write);
ATF_TEST_CASE_HEAD(tc_long_write)
{
    set("descr", "Tests that a long write (one that does not fit in the "
                 "internal buffer) works");
}
ATF_TEST_CASE_BODY(tc_long_write)
{
    test_write(64 * 1024, 1024);
    ATF_PASS();
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&tc_short_read);
    tcs.push_back(&tc_long_read);
    tcs.push_back(&tc_short_write);
    tcs.push_back(&tc_long_write);
}
