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

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "libatf.hpp"

#include "libatfmain/application.hpp"
#include "libatfmain/ui.hpp"

namespace am = atf::main;

class atf_format : public am::application {
    std::string m_tag;

    void process_option(int, const char*);
    std::string specific_args(void) const;
    options_set specific_options(void) const;

public:
    int main(void);
};

typedef std::map< std::string, std::string > confvars_map;

void
atf_format::process_option(int ch, const char* arg)
{
    switch (ch) {
    case 't':
        m_tag = arg;
        break;

    default:
        assert(false);
    }
}

std::string
atf_format::specific_args(void)
    const
{
    return "[str1 [.. strN]]";
}

atf_format::options_set
atf_format::specific_options(void)
    const
{
    options_set opts;
    opts.insert(option('t', true, "Tag to use for printing"));
    return opts;
}

int
atf_format::main(void)
{
    std::string str;

    if (m_argc > 0) {
        for (int i = 0; i < m_argc; i++) {
            str += m_argv[i];
            if (i < m_argc - 1)
                str += ' ';
        }
        str += '\n';
    } else {
        std::string line;
        while (std::getline(std::cin, line))
            str += line + '\n';
    }

    size_t col = 0;
    if (!m_tag.empty()) {
        std::cout << m_tag;
        col += m_tag.length();
    }
    std::cout << am::format_text(str, col, col) << std::endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* const* argv)
{
    return atf_format().run(argc, argv);
}
