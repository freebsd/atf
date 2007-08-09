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
#include <fstream>

#include "atfprivate/atffile.hpp"
#include "atfprivate/exceptions.hpp"
#include "atfprivate/expand.hpp"
#include "atfprivate/serial.hpp"

atf::atffile::atffile(const atf::fs::path& filename)
{
    std::ifstream is(filename.c_str());
    if (!is)
        throw atf::not_found_error< fs::path >
            ("Cannot open Atffile", filename);
    serial::internalizer i(is, "application/X-atf-atffile", 0);

    fs::directory dir(filename.branch_path());
    dir.erase(filename.leaf_name());
    for (fs::directory::iterator iter = dir.begin(); iter != dir.end();
         iter++) {
        const std::string& name = (*iter).first;
        const fs::file_info& fi = (*iter).second;

        if (name[0] == '.' || (!fi.is_owner_executable() &&
                               !fi.is_group_executable()))
            dir.erase(iter);
    }

    std::string line;
    while (std::getline(is, line)) {
        if (expand::is_glob(line)) {
            std::set< std::string > ms =
                expand::expand_glob(line, dir.names());
            insert(end(), ms.begin(), ms.end());
        } else {
            if (dir.find(line) == dir.end())
                throw atf::not_found_error< fs::path >
                    ("Cannot locate the " + line + " file", fs::path(line));
            push_back(line);
        }
    }

    is.close();
}
