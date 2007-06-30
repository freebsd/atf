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
#include <sys/ioctl.h>
}

#include <cassert>
#include <cstdlib>
#include <sstream>

#include "libatfmain/ui.hpp"

static
size_t
terminal_width(void)
{
    static size_t width = 0;

    if (width == 0) {
        const char* cols = std::getenv("COLUMNS");
        if (cols != NULL) {
            std::istringstream str(cols);
            str >> width;
        } else {
            struct winsize ws;
            if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
                width = ws.ws_col;
            else if (::ioctl(STDERR_FILENO, TIOCGWINSZ, &ws) != -1)
                width = ws.ws_col;
            else if (::ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1)
                width = ws.ws_col;
        }

        if (width == 0)
            width = 79;
    }

    assert(width > 0);
    return width;
}

std::vector< std::string >
atf::split(const std::string& str, const std::string& delim)
{
    std::vector< std::string > words;

    std::string::size_type pos = 0, newpos = 0;
    while (pos < str.length() && newpos != std::string::npos) {
        newpos = str.find(delim, pos);
        words.push_back(str.substr(pos, newpos - pos));
        pos = newpos + delim.length();
    }

    return words;
}

// See description for format_text below for more details.
static
std::string
format_paragraph(const std::string& text,
                 const size_t col,
                 size_t curcol)
{
    assert(text.find('\n') == std::string::npos);

    std::string formatted;
    if (curcol < col) {
        formatted = std::string(col - curcol, ' ');
        curcol = col;
    }

    const size_t maxcol = terminal_width();

    std::vector< std::string > words = atf::split(text, " ");
    for (std::vector< std::string >::const_iterator iter = words.begin();
         iter != words.end(); iter++) {
        const std::string& word = *iter;

        if (iter != words.begin() && curcol + word.length() + 1 > maxcol) {
            formatted += '\n' + std::string(col, ' ');
            curcol = col;
        } else if (iter != words.begin()) {
            formatted += ' ';
            curcol++;
        }

        formatted += word;
        curcol += word.length();
    }

    return formatted;
}

// Reformats the given text so that it fits in the current screen with no
// wrapping.
//
// The input text is a series of words and sentences.  Paragraphs may be
// separated with a '\n' character, which is taken into account to do the
// proper formatting.  The text should not finish in '\n'.
//
// 'col' specifies the column where the text will start and 'curcol'
// specifies the current position of the cursor.
std::string
atf::format_text(const std::string& text, const size_t col, size_t curcol)
{
    assert(curcol <= col);

    std::string formatted;

    std::vector< std::string > lines = split(text, "\n");
    for (std::vector< std::string >::const_iterator iter = lines.begin();
         iter != lines.end(); iter++) {
        const std::string& line = *iter;

        formatted += format_paragraph(line, col, curcol);
        if (iter + 1 != lines.end())
            formatted += "\n\n";
        curcol = 0;
    }

    return formatted;
}
