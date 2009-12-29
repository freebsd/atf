//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007, 2008, 2009 The NetBSD Foundation, Inc.
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

#if defined(HAVE_CONFIG_H)
#include "bconfig.h"
#endif

extern "C" {
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
}

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "atf-c++/application.hpp"
#include "atf-c++/atffile.hpp"
#include "atf-c++/config.hpp"
#include "atf-c++/env.hpp"
#include "atf-c++/exceptions.hpp"
#include "atf-c++/formats.hpp"
#include "atf-c++/fs.hpp"
#include "atf-c++/io.hpp"
#include "atf-c++/parser.hpp"
#include "atf-c++/process.hpp"
#include "atf-c++/sanity.hpp"
#include "atf-c++/tests.hpp"
#include "atf-c++/text.hpp"

class config : public atf::formats::atf_config_reader {
    atf::tests::vars_map m_vars;

    void
    got_var(const std::string& var, const std::string& name)
    {
        m_vars[var] = name;
    }

public:
    config(std::istream& is) :
        atf::formats::atf_config_reader(is)
    {
    }

    const atf::tests::vars_map&
    get_vars(void)
        const
    {
        return m_vars;
    }
};

class muxer : public atf::formats::atf_tcs_reader {
    const atf::fs::path m_tp;
    const std::string m_tc;
    atf::tests::tcr m_tcr;
    atf::formats::atf_tps_writer& m_writer;

    bool m_failed;
    bool m_got_tc_end;

    void
    got_ntcs(size_t ntcs)
    {
        if (ntcs != 1)
            throw atf::formats::format_error("Expecting only one test case");
    }

    void
    got_tc_start(const std::string& tcname)
    {
        if (tcname != m_tc)
            throw atf::formats::format_error("Expected " + m_tc +
                                             " but got " + tcname);
    }

    void
    got_tc_end(const atf::tests::tcr& tcr)
    {
        m_got_tc_end = true;
        m_tcr = tcr;
        m_failed = (tcr.get_state() == atf::tests::tcr::failed_state);
    }

    void
    got_stdout_line(const std::string& line)
    {
        m_writer.stdout_tc(line);
    }

    void
    got_stderr_line(const std::string& line)
    {
        m_writer.stderr_tc(line);
    }

public:
    muxer(const atf::fs::path& tp, const std::string& tc,
           atf::formats::atf_tps_writer& w, atf::io::pistream& is) :
        atf::formats::atf_tcs_reader(is),
        m_tp(tp),
        m_tc(tc),
        m_tcr(atf::tests::tcr::failed_state, "UNDEFINED STATE"),
        m_writer(w),
        m_failed(true),
        m_got_tc_end(false)
    {
        m_writer.start_tc(m_tc);
    }

    bool
    failed(void)
        const
    {
        PRE(m_got_tc_end);
        return m_failed;
    }

    bool
    tc_ended(void)
        const
    {
        return m_got_tc_end;
    }

    void
    got_tp_error(const std::string& error)
    {
        m_tcr = atf::tests::tcr(atf::tests::tcr::failed_state, error);
        m_got_tc_end = true;
        m_failed = true;
    }

    ~muxer(void)
    {
        INV(m_got_tc_end);
        m_writer.end_tc(m_tcr);
    }
};

template< class K, class V >
void
merge_maps(std::map< K, V >& dest, const std::map< K, V >& src)
{
    for (typename std::map< K, V >::const_iterator iter = src.begin();
         iter != src.end(); iter++)
        dest[(*iter).first] = (*iter).second;
}

class atf_run : public atf::application::app {
    static const char* m_description;

    atf::tests::vars_map m_atffile_vars;
    atf::tests::vars_map m_cmdline_vars;
    atf::tests::vars_map m_config_vars;

    static atf::tests::vars_map::value_type parse_var(const std::string&);

    void process_option(int, const char*);
    std::string specific_args(void) const;
    options_set specific_options(void) const;

    void parse_vflag(const std::string&);

    void read_one_config(const atf::fs::path&);
    void read_config(const std::string&);
    std::vector< std::string > conf_args(void) const;

    size_t count_tps(std::vector< std::string >) const;

    int run_test(const atf::fs::path&,
                 atf::formats::atf_tps_writer&);
    int run_test_directory(const atf::fs::path&,
                           atf::formats::atf_tps_writer&);
    int run_test_case(const atf::fs::path&, const std::string&,
                      atf::formats::atf_tps_writer&);
    int run_test_program(const atf::fs::path&, atf::formats::atf_tps_writer&);

    std::vector< std::string > query_tcs(const atf::fs::path&) const;

    struct query_tcs_data {
        const atf_run* m_this;
        const atf::fs::path& m_tp;

        query_tcs_data(const atf_run* t, const atf::fs::path& tp) :
            m_this(t),
            m_tp(tp)
        {
        }
    };

    struct test_data {
        const atf_run* m_this;
        const atf::fs::path& m_tp;
        const std::string& m_tc;
        atf::io::pipe& m_respipe;

        test_data(const atf_run* t, const atf::fs::path& tp,
                  const std::string& tc, atf::io::pipe& respipe) :
            m_this(t),
            m_tp(tp),
            m_tc(tc),
            m_respipe(respipe)
        {
        }
    };

    static void route_run_test_case_child(void *);
    void run_test_case_child(const atf::fs::path&, const std::string&,
                                atf::io::pipe&) const;
    int run_test_case_parent(const atf::fs::path&, const std::string&,
                                atf::formats::atf_tps_writer&,
                                atf::process::child&,
                                atf::io::pipe&);

    static void route_query_tcs_child(void *);
    void query_tcs_child(const atf::fs::path&) const;
    std::vector< std::string > query_tcs_parent(const atf::fs::path&,
                                                atf::process::child&) const;

public:
    atf_run(void);

    int main(void);
};

const char* atf_run::m_description =
    "atf-run is a tool that runs tests programs and collects their "
    "results.";

atf_run::atf_run(void) :
    app(m_description, "atf-run(1)", "atf(7)")
{
}

void
atf_run::process_option(int ch, const char* arg)
{
    switch (ch) {
    case 'v':
        parse_vflag(arg);
        break;

    default:
        UNREACHABLE;
    }
}

std::string
atf_run::specific_args(void)
    const
{
    return "[test-program1 .. test-programN]";
}

atf_run::options_set
atf_run::specific_options(void)
    const
{
    using atf::application::option;
    options_set opts;
    opts.insert(option('v', "var=value", "Sets the configuration variable "
                                         "`var' to `value'; overrides "
                                         "values in configuration files"));
    return opts;
}

void
atf_run::parse_vflag(const std::string& str)
{
    if (str.empty())
        throw std::runtime_error("-v requires a non-empty argument");

    std::vector< std::string > ws = atf::text::split(str, "=");
    if (ws.size() == 1 && str[str.length() - 1] == '=') {
        m_cmdline_vars[ws[0]] = "";
    } else {
        if (ws.size() != 2)
            throw std::runtime_error("-v requires an argument of the form "
                                     "var=value");

        m_cmdline_vars[ws[0]] = ws[1];
    }
}

int
atf_run::run_test(const atf::fs::path& tp,
                  atf::formats::atf_tps_writer& w)
{
    atf::fs::file_info fi(tp);

    int errcode;
    if (fi.get_type() == atf::fs::file_info::dir_type)
        errcode = run_test_directory(tp, w);
    else {
        errcode = run_test_program(tp, w);
    }
    return errcode;
}

int
atf_run::run_test_directory(const atf::fs::path& tp,
                            atf::formats::atf_tps_writer& w)
{
    atf::atffile af(tp / "Atffile");
    m_atffile_vars = af.conf();

    atf::tests::vars_map oldvars = m_config_vars;
    {
        atf::tests::vars_map::const_iterator iter =
            af.props().find("test-suite");
        INV(iter != af.props().end());
        read_config((*iter).second);
    }

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = af.tps().begin();
         iter != af.tps().end(); iter++)
        ok &= (run_test(tp / *iter, w) == EXIT_SUCCESS);

    m_config_vars = oldvars;

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

std::vector< std::string >
atf_run::conf_args(void) const
{
    using atf::tests::vars_map;

    atf::tests::vars_map vars;
    std::vector< std::string > args;

    merge_maps(vars, m_atffile_vars);
    merge_maps(vars, m_config_vars);
    merge_maps(vars, m_cmdline_vars);

    for (vars_map::const_iterator i = vars.begin(); i != vars.end(); i++)
        args.push_back("-v" + (*i).first + "=" + (*i).second);

    return args;
}

char** vector_to_argv(const std::vector< std::string >& v)
{
    char** argv = new char*[v.size() + 1];
    for (std::vector< std::string >::size_type i = 0; i < v.size(); i++) {
        argv[i] = strdup(v[i].c_str());
    }
    argv[v.size()] = NULL;
    return argv;
}

void append_to_vector(std::vector< std::string >& v1,
                      const std::vector< std::string >& v2)
{
    std::copy(v2.begin(), v2.end(),
              std::back_insert_iterator< std::vector< std::string > >(v1));
}

void
atf_run::route_run_test_case_child(void* v)
{
    test_data* td = static_cast< test_data* >(v);
    td->m_this->run_test_case_child(td->m_tp, td->m_tc, td->m_respipe);
    UNREACHABLE;
}

void
atf_run::run_test_case_child(const atf::fs::path& tp, const std::string& tc,
                             atf::io::pipe& respipe)
    const
{
    // Remap the results file descriptor to point to the parent too.
    // We use the 9th one (instead of a bigger one) because shell scripts
    // can only use the [0..9] file descriptors in their redirections.
    respipe.rend().close();
    respipe.wend().posix_remap(9);

    // Prepare the test program's arguments.  We use dynamic memory and
    // do not care to release it.  We are going to die anyway very soon,
    // either due to exec(2) or to exit(3).
    std::vector< std::string > argv;
    argv.push_back(tp.leaf_name());
    argv.push_back("-r9");
    argv.push_back("-s" + tp.branch_path().str());
    append_to_vector(argv, conf_args());
    argv.push_back(tc);

    // Do the real exec and report any errors to the parent through the
    // only mechanism we can use: stderr.
    // TODO Try to make this fail.
    ::execv(tp.c_str(), vector_to_argv(argv));
    std::cerr << "Failed to execute `" << tp.str() << "': "
              << std::strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
}

int
atf_run::run_test_case_parent(const atf::fs::path& tp, const std::string& tc,
                              atf::formats::atf_tps_writer& w,
                              atf::process::child& c,
                              atf::io::pipe& respipe)
{
    // Get the input stream of stdout and stderr.
    atf::io::file_handle outfh = c.stdout_fd();
    atf::io::unbuffered_istream outin(outfh);
    atf::io::file_handle errfh = c.stderr_fd();
    atf::io::unbuffered_istream errin(errfh);

    // Get the file descriptor and input stream of the results channel.
    respipe.wend().close();
    atf::io::pistream resin(respipe.rend());

    // Process the test case's output and multiplex it into our output
    // stream as we read it.
    muxer m(tp, tc, w, resin);
    std::string fmterr;
    try {
        m.read(outin, errin);
    } catch (const atf::parser::parse_errors& e) {
        fmterr = "There were errors parsing the output of the test "
                 "program:";
        for (atf::parser::parse_errors::const_iterator iter = e.begin();
             iter != e.end(); iter++) {
            fmterr += " Line " + atf::text::to_string((*iter).first) +
                      ": " + (*iter).second + ".";
        }
    } catch (const atf::formats::format_error& e) {
        fmterr = e.what();
    } catch (...) {
        UNREACHABLE;
    }

    try {
        outin.close();
        errin.close();
        resin.close();
    } catch (...) {
        UNREACHABLE;
    }

    const atf::process::status s = c.wait();

    int code;
    if (s.exited()) {
        code = s.exitstatus();
        if ((!m.tc_ended() || m.failed()) && code == EXIT_SUCCESS) {
            code = EXIT_FAILURE;
            m.got_tp_error("Test program returned success but some test "
                           "cases failed" +
                           (fmterr.empty() ? "" : (".  " + fmterr)));
        } else {
            if (!fmterr.empty())
                m.got_tp_error(fmterr);
        }
    } else if (s.signaled()) {
        code = EXIT_FAILURE;
        m.got_tp_error("Test program received signal " +
                       atf::text::to_string(s.termsig()) +
                       (s.coredump() ? " (core dumped)" : "") +
                       (fmterr.empty() ? "" : (".  " + fmterr)));
    } else
        throw std::runtime_error
            ("Child process " + atf::text::to_string(c.pid()) +
             " terminated with an unknown status condition");
    return code;
}

int
atf_run::run_test_case(const atf::fs::path& tp, const std::string& tc,
                       atf::formats::atf_tps_writer& w)
{
    // XXX: This respipe is quite annoying.  The fact that we cannot
    // represent it as part of a portable fork API (which only supports
    // stdin, stdout and stderr) and not even in our own fork API means
    // that this will be a huge source of portability problems in the
    // future, should we ever want to port ATF to Win32.  I guess it'd
    // be worth revisiting the decision of using a third file descriptor
    // for results reporting sooner than later.  Alternative: use a
    // temporary file.
    atf::io::pipe respipe;
    test_data td(this, tp, tc, respipe);
    atf::process::child c =
        atf::process::fork(route_run_test_case_child,
                           atf::process::stream_capture(),
                           atf::process::stream_capture(),
                           static_cast< void * >(&td));

    return run_test_case_parent(tp, tc, w, c, respipe);
}

int
atf_run::run_test_program(const atf::fs::path& tp,
                          atf::formats::atf_tps_writer& w)
{
    int errcode = EXIT_SUCCESS;

    std::vector< std::string > tcs;
    try {
        tcs = query_tcs(tp);
    } catch (const atf::formats::format_error& e) {
        w.start_tp(tp.str(), 0);
        w.end_tp(e.what());
        return EXIT_FAILURE;
    }

    w.start_tp(tp.str(), tcs.size());
    if (tcs.empty()) {
        w.end_tp("Bogus test program: reported 0 test cases");
        errcode = EXIT_FAILURE;
    } else {
        for (std::vector< std::string >::const_iterator iter = tcs.begin();
             iter != tcs.end(); iter++) {
            if (run_test_case(tp, *iter, w) != EXIT_SUCCESS)
                errcode = EXIT_FAILURE;
        }
        w.end_tp("");
    }

    return errcode;
}

void
atf_run::route_query_tcs_child(void* v)
{
    query_tcs_data* td = static_cast< query_tcs_data* >(v);
    td->m_this->query_tcs_child(td->m_tp);
    UNREACHABLE;
}

void
atf_run::query_tcs_child(const atf::fs::path& tp)
    const
{
    std::vector< std::string > argv;
    argv.push_back(tp.leaf_name());
    argv.push_back("-l");
    argv.push_back("-s" + tp.branch_path().str());
    append_to_vector(argv, conf_args());

    ::execv(tp.c_str(), vector_to_argv(argv));
    std::cerr << "Failed to execute `" << tp.str() << "': "
              << std::strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
}

std::vector< std::string >
atf_run::query_tcs_parent(const atf::fs::path& tp, atf::process::child& child)
    const
{
    // Get the input stream of stdout and stderr.
    atf::io::file_handle outfh = child.stdout_fd();
    atf::io::pistream outin(outfh);

    std::vector< std::string > tcs;

    std::string fmterr;

    std::string line;
    while (getline(outin, line).good()) {
        std::string::size_type pos = line.find(" ");
        if (pos == std::string::npos) {
            fmterr = "Invalid format for test case list in \"" + line + "\"";
            break;
        }

        const std::string tc = line.substr(0, pos);
        tcs.push_back(tc);
    }

    try {
        outin.close();
    } catch (...) {
        UNREACHABLE;
    }

    const atf::process::status status = child.wait();

    if (!fmterr.empty())
        throw atf::formats::format_error(fmterr);
    if (!status.exited() || status.exitstatus() != EXIT_SUCCESS)
        throw atf::formats::format_error("Test program returned failure "
                                         "exit status for test case list");

    return tcs;
}

std::vector< std::string >
atf_run::query_tcs(const atf::fs::path& tp)
    const
{
    query_tcs_data data(this, tp);
    atf::process::child child =
        atf::process::fork(route_query_tcs_child,
                           atf::process::stream_capture(),
                           atf::process::stream_inherit(),
                           static_cast< void * >(&data));

    return query_tcs_parent(tp, child);
}

size_t
atf_run::count_tps(std::vector< std::string > tps)
    const
{
    size_t ntps = 0;

    for (std::vector< std::string >::const_iterator iter = tps.begin();
         iter != tps.end(); iter++) {
        atf::fs::path tp(*iter);
        atf::fs::file_info fi(tp);

        if (fi.get_type() == atf::fs::file_info::dir_type) {
            atf::atffile af = atf::atffile(tp / "Atffile");
            std::vector< std::string > aux = af.tps();
            for (std::vector< std::string >::iterator i2 = aux.begin();
                 i2 != aux.end(); i2++)
                *i2 = (tp / *i2).str();
            ntps += count_tps(aux);
        } else
            ntps++;
    }

    return ntps;
}

void
atf_run::read_one_config(const atf::fs::path& p)
{
    std::ifstream is(p.c_str());
    if (is) {
        config reader(is);
        reader.read();
        merge_maps(m_config_vars, reader.get_vars());
    }
}

void
atf_run::read_config(const std::string& name)
{
    std::vector< atf::fs::path > dirs;
    dirs.push_back(atf::fs::path(atf::config::get("atf_confdir")));
    if (atf::env::has("HOME"))
        dirs.push_back(atf::fs::path(atf::env::get("HOME")) / ".atf");

    m_config_vars.clear();
    for (std::vector< atf::fs::path >::const_iterator iter = dirs.begin();
         iter != dirs.end(); iter++) {
        read_one_config((*iter) / "common.conf");
        read_one_config((*iter) / (name + ".conf"));
    }
}

static
void
call_hook(const std::string& tool, const std::string& hook)
{
    const atf::fs::path sh(atf::config::get("atf_shell"));
    const atf::fs::path hooks =
        atf::fs::path(atf::config::get("atf_pkgdatadir")) / (tool + ".hooks");

    const atf::process::status s =
        atf::process::exec(sh,
                           atf::process::argv_array(sh.c_str(), hooks.c_str(),
                                                    hook.c_str(), NULL),
                           atf::process::stream_inherit(),
                           atf::process::stream_inherit());


    if (!s.exited() || s.exitstatus() != EXIT_SUCCESS)
        throw std::runtime_error("Failed to run the '" + hook + "' hook "
                                 "for '" + tool + "'");
}

int
atf_run::main(void)
{
    atf::atffile af(atf::fs::path("Atffile"));
    m_atffile_vars = af.conf();

    std::vector< std::string > tps;
    tps = af.tps();
    if (m_argc >= 1) {
        // TODO: Ensure that the given test names are listed in the
        // Atffile.  Take into account that the file can be using globs.
        tps.clear();
        for (int i = 0; i < m_argc; i++)
            tps.push_back(m_argv[i]);
    }

    // Read configuration data for this test suite.
    {
        atf::tests::vars_map::const_iterator iter =
            af.props().find("test-suite");
        INV(iter != af.props().end());
        read_config((*iter).second);
    }

    atf::formats::atf_tps_writer w(std::cout);
    call_hook("atf-run", "info_start_hook");
    w.ntps(count_tps(tps));

    bool ok = true;
    for (std::vector< std::string >::const_iterator iter = tps.begin();
         iter != tps.end(); iter++)
        ok &= (run_test(atf::fs::path(*iter), w) == EXIT_SUCCESS);

    call_hook("atf-run", "info_end_hook");

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int
main(int argc, char* const* argv)
{
    return atf_run().run(argc, argv);
}
