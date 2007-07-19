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

#if !defined(_ATF_FS_HPP_)
#define _ATF_FS_HPP_

#include <map>
#include <set>
#include <string>

namespace atf {
namespace fs {

class directory;

class file_info {
public:
    enum type {
        blk_type,
        chr_type,
        dir_type,
        fifo_type,
        lnk_type,
        reg_type,
        sock_type,
        unknown_type,
        wht_type
    };

    const std::string& get_name(void) const;
    type get_type(void) const;

private:
    std::string m_name;
    type m_type;

    explicit file_info(void*);
    friend class directory;
};

class directory : public std::map< std::string, file_info > {
public:
    directory(const std::string& path);
    std::set< std::string > names(void) const;
};

std::string get_branch_path(const std::string&);
std::string get_leaf_name(const std::string&);
std::string get_temp_dir(void);
std::string get_work_dir(void);
bool exists(const std::string&);
std::string create_temp_dir(const std::string&);
void change_directory(const std::string&);
void rm_rf(const std::string&);

} // namespace fs
} // namespace atf

#endif // !defined(_ATF_FS_HPP_)
