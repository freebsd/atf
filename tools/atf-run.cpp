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
#include "atf-c++/signals.hpp"
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

class tp_descr : public atf::formats::atf_tp_reader {
    std::map< std::string, atf::tests::vars_map > m_tcs;

    void got_tc(const std::string& ident,
                const std::map< std::string, std::string >& props)
    {
        if (m_tcs.find(ident) != m_tcs.end())
            throw(std::runtime_error("Duplicate test case " + ident +
                                     " in test program"));
        m_tcs[ident] = props;

        if (m_tcs[ident].find("timeout") == m_tcs[ident].end())
            m_tcs[ident].insert(std::make_pair("timeout", "300"));
    }

public:
    tp_descr(std::istream& is) :
        atf::formats::atf_tp_reader(is)
    {
    }

    const std::map< std::string, atf::tests::vars_map >&
    get_tcs(void)
        const
    {
        return m_tcs;
    }
};

class muxer : public atf::io::std_muxer {
    atf::formats::atf_tps_writer& m_writer;

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
    muxer(atf::formats::atf_tps_writer& w) :
        m_writer(w)
    {
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

    void prepare_child(const atf::fs::path&) const;

    int run_test(const atf::fs::path&,
                 atf::formats::atf_tps_writer&);
    int run_test_directory(const atf::fs::path&,
                           atf::formats::atf_tps_writer&);
    atf::process::status run_test_case(const atf::fs::path&, const std::string&,
                                       const std::string&, const atf::fs::path&,
                                       const atf::fs::path&,
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
        const std::string& m_part;
        const atf::fs::path& m_resfile;
        const atf::fs::path& m_workdir;

        test_data(const atf_run* t, const atf::fs::path& tp,
                  const std::string& tc, const std::string& part,
                  const atf::fs::path& resfile,
                  const atf::fs::path& workdir) :
            m_this(t),
            m_tp(tp),
            m_tc(tc),
            m_part(part),
            m_resfile(resfile),
            m_workdir(workdir)
        {
        }
    };

    atf::tests::tcr get_tcr(const atf::process::status&,
                            const atf::fs::path&) const;

    static void route_run_test_case_child(void *);
    void run_test_case_child(const atf::fs::path&, const std::string&,
                             const std::string&, const atf::fs::path&,
                             const atf::fs::path&) const;
    atf::process::status run_test_case_parent(const atf::fs::path&,
                                              const std::string&,
                                              const atf::fs::path&,
                                              atf::formats::atf_tps_writer&,
                                              atf::process::child&);

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

void
atf_run::prepare_child(const atf::fs::path& workdir)
    const
{
    const int ret = ::setpgid(::getpid(), 0);
    INV(ret != -1);

    umask(S_IWGRP | S_IWOTH);

    for (int i = 1; i <= atf::signals::last_signo; i++)
        atf::signals::reset(i);

    atf::env::set("HOME", workdir.str());
    atf::env::unset("LANG");
    atf::env::unset("LC_ALL");
    atf::env::unset("LC_COLLATE");
    atf::env::unset("LC_CTYPE");
    atf::env::unset("LC_MESSAGES");
    atf::env::unset("LC_MONETARY");
    atf::env::unset("LC_NUMERIC");
    atf::env::unset("LC_TIME");
    atf::env::unset("TZ");

    atf::fs::change_directory(workdir);
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
    td->m_this->run_test_case_child(td->m_tp, td->m_tc, td->m_part,
                                    td->m_resfile, td->m_workdir);
    UNREACHABLE;
}

void
atf_run::run_test_case_child(const atf::fs::path& tp, const std::string& tc,
                             const std::string& part,
                             const atf::fs::path& resfile,
                             const atf::fs::path& workdir)
    const
{
    // The input 'tp' parameter may be relative and become invalid once
    // we change the current working directory.
    const atf::fs::path absolute_tp = tp.to_absolute();

    // Prepare the test program's arguments.  We use dynamic memory and
    // do not care to release it.  We are going to die anyway very soon,
    // either due to exec(2) or to exit(3).
    std::vector< std::string > argv;
    argv.push_back(tp.leaf_name());
    argv.push_back("-r" + resfile.str());
    argv.push_back("-s" + absolute_tp.branch_path().str());
    append_to_vector(argv, conf_args());
    argv.push_back(tc + ":" + part);

    prepare_child(workdir);

    // Do the real exec and report any errors to the parent through the
    // only mechanism we can use: stderr.
    // TODO Try to make this fail.
    ::execv(absolute_tp.c_str(), vector_to_argv(argv));
    std::cerr << "Failed to execute `" << tp.str() << "': "
              << std::strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
}

atf::tests::tcr
atf_run::get_tcr(const atf::process::status& s,
                 const atf::fs::path& resfile)
    const
{
    using atf::tests::tcr;

    if (s.exited()) {
        try {
            const tcr ret = tcr::read(resfile);
            if (ret.get_state() == tcr::failed_state) {
                if (s.exitstatus() == EXIT_SUCCESS)
                    return tcr(tcr::failed_state, "Test case exited "
                               "successfully but reported failure");
            } else {
                if (s.exitstatus() != EXIT_SUCCESS)
                    return tcr(tcr::failed_state, "Test case exited "
                               "with error but reported success");
            }
            return ret;
        } catch (const atf::formats::format_error& e) {
            return tcr(tcr::failed_state, "Test case created a bogus results "
                       "file: " + std::string(e.what()));
        } catch (const atf::parser::parse_errors& e) {
            std::string reason = "Test case created a bogus results file: ";
            //for (std::vector< atf::parser::parse_error >::const_iterator iter =
                 //e.begin(); iter != e.end(); iter++) {
                //reason += (*iter).what();
            //}
            reason += atf::text::join(e, "; ");
            return tcr(tcr::failed_state, reason);
        } catch (const std::runtime_error& e) {
            return tcr(tcr::failed_state, "Test case exited normally but "
                       "failed to create the results file: " +
                       std::string(e.what()));
        }
    } else if (s.signaled()) {
        return tcr(tcr::failed_state,
                   "Test program received signal " +
                   atf::text::to_string(s.termsig()) +
                   (s.coredump() ? " (core dumped)" : ""));
    } else {
        UNREACHABLE;
        throw std::runtime_error("Unknown exit status");
    }
}

atf::process::status
atf_run::run_test_case_parent(const atf::fs::path& tp, const std::string& tc,
                              const atf::fs::path& workdir,
                              atf::formats::atf_tps_writer& w,
                              atf::process::child& c)
{
    // Get the input stream of stdout and stderr.
    atf::io::file_handle outfh = c.stdout_fd();
    atf::io::unbuffered_istream outin(outfh);
    atf::io::file_handle errfh = c.stderr_fd();
    atf::io::unbuffered_istream errin(errfh);

    // Process the test case's output and multiplex it into our output
    // stream as we read it.
    try {
        muxer m(w);
        m.read(outin, errin);
        outin.close();
        errin.close();
    } catch (...) {
        UNREACHABLE;
    }

    return c.wait();
}

atf::process::status
atf_run::run_test_case(const atf::fs::path& tp, const std::string& tc,
                       const std::string& part, const atf::fs::path& resfile,
                       const atf::fs::path& workdir,
                       atf::formats::atf_tps_writer& w)
{
    // TODO: Capture termination signals and deliver them to the subprocess
    // instead.  Or maybe do something else; think about it.

    test_data td(this, tp, tc, part, resfile, workdir);
    atf::process::child c =
        atf::process::fork(route_run_test_case_child,
                           atf::process::stream_capture(),
                           atf::process::stream_capture(),
                           static_cast< void * >(&td));

    return run_test_case_parent(tp, tc, workdir, w, c);
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
        w.end_tp("Invalid format for test case list: " + std::string(e.what()));
        return EXIT_FAILURE;
    } catch (const atf::parser::parse_errors& e) {
        const std::string reason = atf::text::join(e, "; ");
        w.start_tp(tp.str(), 0);
        w.end_tp("Invalid format for test case list: " + reason);
        return EXIT_FAILURE;
    }

    atf::fs::temp_dir resdir(atf::fs::path(atf::config::get("atf_workdir")) /
                             "atf-run.XXXXXX");

    w.start_tp(tp.str(), tcs.size());
    if (tcs.empty()) {
        w.end_tp("Bogus test program: reported 0 test cases");
        errcode = EXIT_FAILURE;
    } else {
        for (std::vector< std::string >::const_iterator iter = tcs.begin();
             iter != tcs.end(); iter++) {
            const atf::fs::path resfile = resdir.get_path() / "tcr";
            try {
                w.start_tc(*iter);

                atf::fs::temp_dir workdir(atf::fs::path(atf::config::get("atf_workdir")) /
                                          "atf-run.XXXXXX");

                const atf::process::status body_status =
                    run_test_case(tp, *iter, "body", resfile,
                    workdir.get_path(), w);
                const atf::process::status cleanup_status =
                    run_test_case(tp, *iter, "cleanup", resfile,
                    workdir.get_path(), w);

                // TODO: Force deletion of workdir.

                const atf::tests::tcr tcr = get_tcr(body_status, resfile);
                w.end_tc(tcr);
                if (tcr.get_state() == atf::tests::tcr::failed_state)
                    errcode = EXIT_FAILURE;
            } catch (...) {
                atf::fs::remove(resfile);
                throw;
            }
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

    tp_descr parser(outin);
    parser.read();

    const atf::process::status status = child.wait();

    const std::map< std::string, atf::tests::vars_map >& tcs = parser.get_tcs();
    std::vector< std::string > idents;
    for (std::map< std::string, atf::tests::vars_map >::const_iterator iter =
         tcs.begin(); iter != tcs.end(); iter++) {
        idents.push_back((*iter).first);
    }

    if (!status.exited() || status.exitstatus() != EXIT_SUCCESS)
        throw atf::formats::format_error("Test program returned failure "
                                         "exit status for test case list");

    return idents;
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
