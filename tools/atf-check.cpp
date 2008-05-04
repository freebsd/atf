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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <utility>

#include "atf-c++/application.hpp"
#include "atf-c++/exceptions.hpp"
#include "atf-c++/check.hpp"
#include "atf-c++/fs.hpp"
#include "atf-c++/sanity.hpp"
#include "atf-c++/text.hpp"


class atf_check : public atf::application::app {
    enum output_check_t {
        OC_IGNORE,
        OC_INLINE,
        OC_FILE,
        OC_NULL,
        OC_SAVE
    };

    enum status_check_t {
        SC_EQUAL,
        SC_NOT_EQUAL,
        SC_IGNORE
    };

    output_check_t m_stdout_check, m_stderr_check;
    std::string m_stdout_arg, m_stderr_arg;

    status_check_t m_status_check;
    int m_status_arg;

    static const char* m_description;

    bool empty_file(const std::string &);
    bool equal_file_file(const std::string &, const std::string &);
    bool equal_file_string(const std::string &, const std::string &);

    bool run_status_check(const atf_cmd_result &);
    bool run_stdout_check(const atf_cmd_result &);
    bool run_stderr_check(const atf_cmd_result &);

    std::string specific_args(void) const;
    options_set specific_options(void) const;
    void process_option(int, const char *);
    void process_option_s(const std::string &);
    void process_option_o(const std::string &);
    void process_option_e(const std::string &);

public:
    atf_check(void);
    int main(void);
};

const char* atf_check::m_description =
    "atf-check executes given command and analyzes its results.";

atf_check::atf_check(void) :
    app(m_description, "atf-check(1)", "atf(7)"),
    m_stdout_check(OC_NULL),
    m_stderr_check(OC_NULL),
    m_status_check(SC_EQUAL),
    m_status_arg(0)
{
}

bool
atf_check::empty_file(const std::string &path)
{
    atf::fs::path p(path);
    atf::fs::file_info f(p);

    return (f.get_size() == 0);
}

bool
atf_check::equal_file_file(const std::string &path1, const std::string &path2)
{
    atf_error_t err;
    bool ret = atf_equal_file_file(&err, path1.c_str(), path2.c_str());

    if (atf_is_error(err))
        atf::throw_atf_error(err);

    return ret;
}

bool
atf_check::equal_file_string(const std::string &path, const std::string &str)
{
    atf_error_t err;
    bool ret = atf_equal_file_string(&err, path.c_str(), str.c_str());

    if (atf_is_error(err))
        atf::throw_atf_error(err);

    return ret;
}

bool
atf_check::run_status_check(const atf_cmd_result &r)
{
    int status = atf_cmd_result_status(&r);

    if (m_status_check == SC_EQUAL) {

        if (m_status_arg != status) {
            std::cerr << "Fail: expected exit status " << m_status_arg
                      << " but got " << status << std::endl;
            return false;
        }

    } else if (m_status_check == SC_NOT_EQUAL) {

        if (m_status_arg == status) {
            std::cerr << "Fail: expected exit status other than "
                      << m_status_arg << std::endl;
            return false;
        }
    }

    return true;
}

bool
atf_check::run_stdout_check(const atf_cmd_result &r)
{
    const char *path = atf_fs_path_cstring(atf_cmd_result_stdout(&r));

    if (m_stdout_check == OC_NULL) {

        if (!empty_file(path)) {
            std::cerr << "Fail: command's stdout was not empty" << std::endl;
            return false;
        }

    } else if (m_stdout_check == OC_FILE) {

        if (!equal_file_file(path, m_stdout_arg)) {
            std::cerr << "Fail: command's stdout and file '" << m_stdout_arg
                      << "' differ" << std::endl;
            return false;
        }

    } else if (m_stdout_check == OC_INLINE) {

        if (!equal_file_string(path, m_stdout_arg)) {
            std::cerr << "Fail: command's stdout and '" << m_stdout_arg
                      << "' differ" << std::endl;
            return false;
        }

    } else if (m_stdout_check == OC_SAVE) {

        std::ifstream ifs(path, std::fstream::binary);
        std::ofstream ofs(m_stdout_arg.c_str(), std::fstream::binary);
        ofs << ifs.rdbuf();
        
    }
    
    return true;
}

bool
atf_check::run_stderr_check(const atf_cmd_result &r)
{
    const char *path = atf_fs_path_cstring(atf_cmd_result_stderr(&r));

    if (m_stderr_check == OC_NULL) {

        if (!empty_file(path)) {
            std::cerr << "Fail: command's stderr was not empty" << std::endl;
            return false;
        }

    } else if (m_stderr_check == OC_FILE) {

        if (!equal_file_file(path, m_stderr_arg)) {
            std::cerr << "Fail: command's stderr and file '" << m_stderr_arg
                      << "' differ" << std::endl;
            return false;
        }

    } else if (m_stderr_check == OC_INLINE) {

        if (!equal_file_string(path, m_stderr_arg)) {
            std::cerr << "Fail: command's stderr and '" << m_stderr_arg
                      << "' differ" << std::endl;
            return false;
        }

    } else if (m_stderr_check == OC_SAVE) {

        std::ifstream ifs(path, std::fstream::binary);
        std::ofstream ofs(m_stderr_arg.c_str(), std::fstream::binary);
        ofs << ifs.rdbuf();
        
    }
    
    return true;
}

std::string
atf_check::specific_args(void)
    const
{
    return "<command>";
}

atf_check::options_set
atf_check::specific_options(void)
    const
{
    using atf::application::option;
    options_set opts;

    opts.insert(option('s', "action:value", "Handle status. Action "
                        "must be one of: eq:<num> ne:<num>"));
    opts.insert(option('o', "action:value", "Handle stdout. Action must be "
                        "one of: ignore file:<path> inline:<val> save:<path>"));
    opts.insert(option('e', "action:value", "Handle stderr. Action must be "
                        "one of: ignore file:<path> inline:<val> save:<path>"));

    return opts;
}

void
atf_check::process_option_s(const std::string& arg)
{
    using atf::application::usage_error;


    if (arg == "ignore") {
        m_status_check = SC_IGNORE;
        return;
    }

    std::string::size_type pos = arg.find(':');
    std::string action = arg.substr(0, pos);

    if (action == "eq")
        m_status_check = SC_EQUAL;
    else if (action == "ne")
        m_status_check = SC_NOT_EQUAL;
    else
        throw usage_error("Invalid value for -s option");


    std::string value = arg.substr(pos + 1);

    try {
        m_status_arg = atf::text::to_type< unsigned int >(value);
    } catch (std::runtime_error) {
        throw usage_error("Invalid value for -s option; must be an "
                          "integer in range 0-255");
    }

}

void
atf_check::process_option_o(const std::string& arg)
{
    using atf::application::usage_error;

    std::string::size_type pos = arg.find(':');
    std::string action = arg.substr(0, pos);

    if (action == "ignore")
        m_stdout_check = OC_IGNORE;
    else if (action == "save")
        m_stdout_check = OC_SAVE;
    else if (action == "inline")
        m_stdout_check = OC_INLINE;
    else if (action == "file")
        m_stdout_check = OC_FILE;
    else
        throw usage_error("Invalid value for -o option");

    if (m_stdout_check != OC_IGNORE)
        m_stdout_arg = arg.substr(pos + 1);
}

void
atf_check::process_option_e(const std::string& arg)
{
    using atf::application::usage_error;

    std::string::size_type pos = arg.find(':');
    std::string action = arg.substr(0, pos);

    if (action == "ignore")
        m_stderr_check = OC_IGNORE;
    else if (action == "save")
        m_stderr_check = OC_SAVE;
    else if (action == "inline")
        m_stderr_check = OC_INLINE;
    else if (action == "file")
        m_stderr_check = OC_FILE;
    else
        throw usage_error("Invalid value for -o option");

    if (m_stderr_check != OC_IGNORE)
        m_stderr_arg = arg.substr(pos + 1);
}

void
atf_check::process_option(int ch, const char* arg)
{
    switch (ch) {
    case 's':
        process_option_s(arg);
        break;

    case 'o':
        process_option_o(arg);
        break;

    case 'e':
        process_option_e(arg);
        break;

    default:
        UNREACHABLE;
    }
}

int
atf_check::main(void)
{
    if (m_argc < 1)
        throw atf::application::usage_error("No command specified");

    int status = EXIT_FAILURE;

    atf_cmd_result_t r;
    atf_error_t err;

    err = atf_cmd_run(&r, m_argv[0]);
    if (atf_is_error(err))
        atf::throw_atf_error(err);

    try {
        if ((run_status_check(r) == false) ||
            (run_stdout_check(r) == false) ||
            (run_stderr_check(r) == false))
            status = EXIT_FAILURE;
        else
            status = EXIT_SUCCESS;
    } catch (...) {
        atf_cmd_result_fini(&r);
        throw;
    }

    atf_cmd_result_fini(&r);
    return status;
}

int
main(int argc, char* const* argv)
{
    return atf_check().run(argc, argv);
}
