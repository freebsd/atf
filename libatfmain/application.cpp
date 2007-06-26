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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "libatfmain/application.hpp"
#include "libatfmain/exceptions.hpp"

namespace am = atf::main;

const char* am::application::m_prog_name = NULL;

am::application::option::option(char ch, bool a, const std::string& desc) :
    m_character(ch),
    m_argument(a),
    m_description(desc)
{
}

bool
am::application::option::operator<(const am::application::option& o)
    const
{
    return m_character < o.m_character;
}

am::application::application(void) :
    m_argc(-1),
    m_argv(NULL)
{
}

am::application::~application(void)
{
}

bool
am::application::inited(void)
{
    return m_argc != -1;
}

am::application::options_set
am::application::options(void)
{
    options_set opts = specific_options();
    opts.insert(option('h', false, "Shows this help message"));
    return opts;
}

std::string
am::application::specific_args(void)
    const
{
    return "";
}

am::application::options_set
am::application::specific_options(void)
    const
{
    return options_set();
}

void
am::application::process_option(int ch, const char* arg)
{
}

void
am::application::process_options(void)
{
    assert(inited());

    std::string optstr(":");
    {
        options_set opts = options();
        for (options_set::const_iterator iter = opts.begin();
             iter != opts.end(); iter++) {
            const option& opt = (*iter);

            optstr += opt.m_character;
            if (opt.m_argument)
                optstr += ':';
        }
    }

    int ch;
    ::opterr = 0;
    while ((ch = ::getopt(m_argc, m_argv, optstr.c_str())) != -1) {
        switch (ch) {
            case 'h':
                usage(std::cout);
                ::exit(EXIT_SUCCESS);

            case ':':
                throw usage_error("Option -%c requires an argument.",
                                  ::optopt);

            case '?':
                throw usage_error("Unknown option -%c.", ::optopt);

            default:
                process_option(ch, ::optarg);
        }
    }
    m_argc -= ::optind;
    m_argv += ::optind;
}

void
am::application::usage(std::ostream& os)
{
    assert(inited());

    os << "Usage: " << m_prog_name << " [options]";
    std::string args = specific_args();
    if (!args.empty())
        os << " " << args;
    os << std::endl << std::endl;

    os << "Available options:" << std::endl;
    options_set opts = options();
    for (options_set::const_iterator iter = opts.begin();
         iter != opts.end(); iter++) {
        const option& opt = (*iter);

        os << "    -" << opt.m_character;
        if (opt.m_argument)
            os << " arg";
        else
            os << "    ";
        os << "    " << opt.m_description << std::endl;
    }
}

int
am::application::run(int argc, char* const* argv)
{
    assert(argc > 0);
    assert(argv != NULL);

    m_argc = argc;
    m_argv = argv;

    m_prog_name = std::strrchr(m_argv[0], '/');
    if (m_prog_name == NULL)
        m_prog_name = m_argv[0];
    else
        m_prog_name++;

    try {
        process_options();
        return main();
    } catch (const usage_error& e) {
        std::cerr << "Syntax error: " << e.what() << std::endl
                  << "Type `" << m_prog_name << " -h' for more details."
                  << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Caught unexpected error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Caught unknown error!" << std::endl;
        return EXIT_FAILURE;
    }
}
