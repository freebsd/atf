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
#include <getopt.h>
#include <unistd.h>
}

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "libatf.hpp"

atf::test_suite init_test_suite(void);

namespace atf {
namespace detail {

static const char* prog_name;

class usage_error : public std::runtime_error {
    char m_text[4096];

public:
    usage_error(const char* fmt, ...) throw();

    virtual ~usage_error(void) throw();

    const char* what(void) const throw();
};

static void usage(std::ostream&);
static int safe_main(int, char *[]);
static void set_prog_name(const char*);

} // namespace detail
} // namespace atf

namespace atfd = atf::detail;

// ------------------------------------------------------------------------

atfd::usage_error::usage_error(const char *fmt, ...)
    throw() :
    std::runtime_error("usage_error; message unformatted")
{
    va_list ap;

    va_start(ap, fmt);
    std::vsnprintf(m_text, sizeof(m_text), fmt, ap);
    va_end(ap);
}

atfd::usage_error::~usage_error(void)
    throw()
{
}

const char*
atfd::usage_error::what(void)
    const throw()
{
    return m_text;
}

static
void
atfd::set_prog_name(const char* arg)
{
    prog_name = std::strrchr(arg, '/');
    if (prog_name == NULL)
        prog_name = arg;
    else
        prog_name++;
}

static
void
atfd::usage(std::ostream& os)
{
    os << "Usage: " << atfd::prog_name << " [OPTIONS]" << std::endl
       << std::endl
       << "Available options:" << std::endl
       << "    -h    Shows this help message." << std::endl;
}

int
atfd::safe_main(int argc, char *argv[])
{
    int ch;

    ::opterr = 0;
    while ((ch = ::getopt(argc, argv, ":h")) != -1) {
        switch (ch) {
            case 'h':
                usage(std::cout);
                return EXIT_SUCCESS;

            case ':':
                throw usage_error("Option -%c requires an argument.",
                                  ::optopt);

            case '?':
            default:
                throw usage_error("Unknown option -%c.", ::optopt);
        }
    }
    argc -= ::optind;
    argv += ::optind;

    atf::report r(std::cout);
    atf::test_suite ts = init_test_suite();
    ts.run(&r);

    return EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
    atfd::set_prog_name(argv[0]);

    try {
        return atfd::safe_main(argc, argv);
    } catch (const atfd::usage_error& e) {
        std::cerr << "Syntax error: " << e.what() << std::endl
                  << "Type `" << atfd::prog_name << " -h' for more details."
                  << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Caught unexpected error!" << std::endl;
        return EXIT_FAILURE;
    }
}
