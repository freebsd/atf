//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007, 2008, 2009, 2010 The NetBSD Foundation, Inc.
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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
}

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>

#include "../macros.hpp"

#include "io.hpp"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

static
void
systembuf_check_data(std::istream& is, std::size_t length)
{
    char ch = 'A', chr;
    std::size_t cnt = 0;
    while (is >> chr) {
        ATF_REQUIRE_EQ(ch, chr);
        if (ch == 'Z')
            ch = 'A';
        else
            ch++;
        cnt++;
    }
    ATF_REQUIRE_EQ(cnt, length);
}

static
void
systembuf_write_data(std::ostream& os, std::size_t length)
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
systembuf_test_read(std::size_t length, std::size_t bufsize)
{
    using atf::io::systembuf;

    std::ofstream f("test_read.txt");
    systembuf_write_data(f, length);
    f.close();

    int fd = ::open("test_read.txt", O_RDONLY);
    ATF_REQUIRE(fd != -1);
    systembuf sb(fd, bufsize);
    std::istream is(&sb);
    systembuf_check_data(is, length);
    ::close(fd);
    ::unlink("test_read.txt");
}

static
void
systembuf_test_write(std::size_t length, std::size_t bufsize)
{
    using atf::io::systembuf;

    int fd = ::open("test_write.txt", O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    ATF_REQUIRE(fd != -1);
    systembuf sb(fd, bufsize);
    std::ostream os(&sb);
    systembuf_write_data(os, length);
    ::close(fd);

    std::ifstream is("test_write.txt");
    systembuf_check_data(is, length);
    is.close();
    ::unlink("test_write.txt");
}

// ------------------------------------------------------------------------
// Test cases for the "file_handle" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(file_handle_ctor);
ATF_TEST_CASE_HEAD(file_handle_ctor)
{
    set_md_var("descr", "Tests file_handle's constructors");
}
ATF_TEST_CASE_BODY(file_handle_ctor)
{
    using atf::io::file_handle;

    file_handle fh1;
    ATF_REQUIRE(!fh1.is_valid());

    file_handle fh2(STDOUT_FILENO);
    ATF_REQUIRE(fh2.is_valid());
    fh2.disown();
}

ATF_TEST_CASE(file_handle_copy);
ATF_TEST_CASE_HEAD(file_handle_copy)
{
    set_md_var("descr", "Tests file_handle's copy constructor");
}
ATF_TEST_CASE_BODY(file_handle_copy)
{
    using atf::io::file_handle;

    file_handle fh1;
    file_handle fh2(STDOUT_FILENO);

    file_handle fh3(fh2);
    ATF_REQUIRE(!fh2.is_valid());
    ATF_REQUIRE(fh3.is_valid());

    fh1 = fh3;
    ATF_REQUIRE(!fh3.is_valid());
    ATF_REQUIRE(fh1.is_valid());

    fh1.disown();
}

ATF_TEST_CASE(file_handle_get);
ATF_TEST_CASE_HEAD(file_handle_get)
{
    set_md_var("descr", "Tests the file_handle::get method");
}
ATF_TEST_CASE_BODY(file_handle_get)
{
    using atf::io::file_handle;

    file_handle fh1(STDOUT_FILENO);
    ATF_REQUIRE_EQ(fh1.get(), STDOUT_FILENO);
}

ATF_TEST_CASE(file_handle_posix_remap);
ATF_TEST_CASE_HEAD(file_handle_posix_remap)
{
    set_md_var("descr", "Tests the file_handle::posix_remap method");
}
ATF_TEST_CASE_BODY(file_handle_posix_remap)
{
    using atf::io::file_handle;

    int pfd[2];

    ATF_REQUIRE(::pipe(pfd) != -1);
    file_handle rend(pfd[0]);
    file_handle wend(pfd[1]);

    ATF_REQUIRE(rend.get() != 10);
    ATF_REQUIRE(wend.get() != 10);
    wend.posix_remap(10);
    ATF_REQUIRE_EQ(wend.get(), 10);
    ATF_REQUIRE(::write(wend.get(), "test-posix-remap", 16) != -1);
    {
        char buf[17];
        ATF_REQUIRE_EQ(::read(rend.get(), buf, sizeof(buf)), 16);
        buf[16] = '\0';
        ATF_REQUIRE(std::strcmp(buf, "test-posix-remap") == 0);
    }

    // Redo previous to ensure that remapping over the same descriptor
    // has no side-effects.
    ATF_REQUIRE_EQ(wend.get(), 10);
    wend.posix_remap(10);
    ATF_REQUIRE_EQ(wend.get(), 10);
    ATF_REQUIRE(::write(wend.get(), "test-posix-remap", 16) != -1);
    {
        char buf[17];
        ATF_REQUIRE_EQ(::read(rend.get(), buf, sizeof(buf)), 16);
        buf[16] = '\0';
        ATF_REQUIRE(std::strcmp(buf, "test-posix-remap") == 0);
    }
}

// ------------------------------------------------------------------------
// Test cases for the "systembuf" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(systembuf_short_read);
ATF_TEST_CASE_HEAD(systembuf_short_read)
{
    set_md_var("descr", "Tests that a short read (one that fits in the "
               "internal buffer) works when using systembuf");
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(systembuf_short_read)
{
    systembuf_test_read(64, 1024);
}

ATF_TEST_CASE(systembuf_long_read);
ATF_TEST_CASE_HEAD(systembuf_long_read)
{
    set_md_var("descr", "Tests that a long read (one that does not fit in "
               "the internal buffer) works when using systembuf");
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(systembuf_long_read)
{
    systembuf_test_read(64 * 1024, 1024);
}

ATF_TEST_CASE(systembuf_short_write);
ATF_TEST_CASE_HEAD(systembuf_short_write)
{
    set_md_var("descr", "Tests that a short write (one that fits in the "
               "internal buffer) works when using systembuf");
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(systembuf_short_write)
{
    systembuf_test_write(64, 1024);
}

ATF_TEST_CASE(systembuf_long_write);
ATF_TEST_CASE_HEAD(systembuf_long_write)
{
    set_md_var("descr", "Tests that a long write (one that does not fit "
               "in the internal buffer) works when using systembuf");
    set_md_var("use.fs", "true");
}
ATF_TEST_CASE_BODY(systembuf_long_write)
{
    systembuf_test_write(64 * 1024, 1024);
}

// ------------------------------------------------------------------------
// Test cases for the "pistream" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(pistream);
ATF_TEST_CASE_HEAD(pistream)
{
    set_md_var("descr", "Tests the pistream class");
}
ATF_TEST_CASE_BODY(pistream)
{
    using atf::io::file_handle;
    using atf::io::pistream;
    using atf::io::systembuf;

    int fds[2];
    ATF_REQUIRE(::pipe(fds) != -1);

    pistream rend(fds[0]);

    systembuf wbuf(fds[1]);
    std::ostream wend(&wbuf);

    // XXX This assumes that the pipe's buffer is big enough to accept
    // the data written without blocking!
    wend << "1Test 1message\n";
    wend.flush();
    std::string tmp;
    rend >> tmp;
    ATF_REQUIRE_EQ(tmp, "1Test");
    rend >> tmp;
    ATF_REQUIRE_EQ(tmp, "1message");
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the tests for the "file_handle" class.
    ATF_ADD_TEST_CASE(tcs, file_handle_ctor);
    ATF_ADD_TEST_CASE(tcs, file_handle_copy);
    ATF_ADD_TEST_CASE(tcs, file_handle_get);
    ATF_ADD_TEST_CASE(tcs, file_handle_posix_remap);

    // Add the tests for the "systembuf" class.
    ATF_ADD_TEST_CASE(tcs, systembuf_short_read);
    ATF_ADD_TEST_CASE(tcs, systembuf_long_read);
    ATF_ADD_TEST_CASE(tcs, systembuf_short_write);
    ATF_ADD_TEST_CASE(tcs, systembuf_long_write);

    // Add the tests for the "pistream" class.
    ATF_ADD_TEST_CASE(tcs, pistream);
}
