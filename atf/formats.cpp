//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007 The NetBSD Foundation, Inc.
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
#include <poll.h>
}

#include <cassert>
#include <iostream>
#include <sstream>

#include "atf/formats.hpp"

namespace impl = atf::formats;
#define IMPL_NAME "atf::formats"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

static
size_t
read_size_t(atf::serial::internalizer& is, const std::string& err)
{
    if (!is.good())
        throw atf::serial::format_error(err);

    std::string line;
    is.getline(line);

    std::istringstream ss(line);
    size_t s;
    ss >> s;

    return s;
}

static
atf::tests::tcr::status
string_to_tcr_status(const std::string& s)
{
    if (s == "passed")
        return atf::tests::tcr::status_passed;
    else if (s == "failed")
        return atf::tests::tcr::status_failed;
    else if (s == "skipped")
        return atf::tests::tcr::status_skipped;
    else
        throw atf::serial::format_error("Invalid test case status `" + s +
                                        "'");
}

static
atf::tests::tcr
read_tcr(atf::serial::internalizer& is, const std::string& firstline)
{
    std::string line;

    atf::tests::tcr tcr;
    {
        switch (string_to_tcr_status(firstline)) {
        case atf::tests::tcr::status_passed:
            tcr = atf::tests::tcr::passed();
            break;

        case atf::tests::tcr::status_failed:
            if (!is.good())
                throw atf::serial::format_error("Missing reason for "
                                                "failed test case result");

            is.getline(line);
            tcr = atf::tests::tcr::failed(line);
            break;

        case atf::tests::tcr::status_skipped:
            if (!is.good())
                throw atf::serial::format_error("Missing reason for "
                                                "skipped test case result");

            is.getline(line);
            tcr = atf::tests::tcr::skipped(line);
            break;

        default:
            assert(false);
        }
    }
    return tcr;
}

static
atf::tests::tcr
read_tcr(atf::serial::internalizer& is)
{
    if (!is.good())
        throw atf::serial::format_error("Missing test case result");

    atf::tests::tcr tcr;
    {
        std::string line;
        is.getline(line);
        tcr = read_tcr(is, line);
    }
    return tcr;
}

static
void
read_terminator(atf::serial::internalizer& is, const std::string& err)
{
    if (!is.good())
        throw atf::serial::format_error(err);

    std::string line;
    is.getline(line);
    if (line != ".")
        throw atf::serial::format_error(err);
}

// ------------------------------------------------------------------------
// The "atf_atffile_reader" class.
// ------------------------------------------------------------------------

impl::atf_atffile_reader::atf_atffile_reader(std::istream& is) :
    m_int(is, "application/X-atf-atffile", 0)
{
}

impl::atf_atffile_reader::~atf_atffile_reader(void)
{
}

void
impl::atf_atffile_reader::got_tp(const std::string& name)
{
}

void
impl::atf_atffile_reader::got_ts(const std::string& name)
{
}

void
impl::atf_atffile_reader::got_var(const std::string& var,
                                  const std::string& val)
{
}

void
impl::atf_atffile_reader::got_eof(void)
{
}

void
impl::atf_atffile_reader::read(void)
{
    using atf::serial::format_error;

    bool seents = false;
    std::string line;
    while (m_int.getline(line).good()) {
        if (line.empty())
            continue;

        if (line[0] == '#')
            continue;

        std::string::size_type pos = line.find('=');
        if (pos != std::string::npos)
            got_var(line.substr(0, pos), line.substr(pos + 1));
        else {
            std::string::size_type pos2 = line.find(": ");
            if (pos2 != std::string::npos) {
                const std::string& name = line.substr(0, pos2);
                const std::string& val = line.substr(pos2 + 2);

                if (name == "test-suite") {
                    got_ts(val);
                    seents = true;
                } else
                    throw format_error("Unknown meta-data keyword " + name);
            } else {
                got_tp(line);
            }
        }
    }

    if (!seents)
        throw format_error("Test suite name not defined");

    got_eof();
}

// ------------------------------------------------------------------------
// The "atf_config_reader" class.
// ------------------------------------------------------------------------

impl::atf_config_reader::atf_config_reader(std::istream& is) :
    m_int(is, "application/X-atf-config", 0)
{
}

impl::atf_config_reader::~atf_config_reader(void)
{
}

void
impl::atf_config_reader::got_var(const std::string& var,
                                 const std::string& val)
{
}

void
impl::atf_config_reader::got_eof(void)
{
}

void
impl::atf_config_reader::read(void)
{
    using atf::serial::format_error;

    std::string line;
    while (m_int.getline(line).good()) {
        if (line.empty())
            continue;

        if (line[0] == '#')
            continue;

        std::string::size_type pos = line.find('=');
        if (pos == std::string::npos)
            throw format_error("Syntax error: invalid variable "
                               "definition `" + line + "'");

        got_var(line.substr(0, pos), line.substr(pos + 1));
    }

    got_eof();
}

// ------------------------------------------------------------------------
// The "atf_tcs_reader" class.
// ------------------------------------------------------------------------

impl::atf_tcs_reader::atf_tcs_reader(std::istream& is) :
    m_int(is, "application/X-atf-tcs", 0)
{
}

impl::atf_tcs_reader::~atf_tcs_reader(void)
{
}

void
impl::atf_tcs_reader::got_ntcs(size_t ntcs)
{
}

void
impl::atf_tcs_reader::got_tc_start(const std::string& tcname)
{
}

void
impl::atf_tcs_reader::got_tc_end(const atf::tests::tcr& tcr)
{
}

void
impl::atf_tcs_reader::got_stdout_line(const std::string& line)
{
}

void
impl::atf_tcs_reader::got_stderr_line(const std::string& line)
{
}

void
impl::atf_tcs_reader::got_eof(void)
{
}

void
impl::atf_tcs_reader::read_out_err(atf::io::unbuffered_istream& out,
                                   atf::io::unbuffered_istream& err)
{
    struct pollfd fds[2];
    fds[0].fd = out.get_fh().get();
    fds[0].events = POLLIN;
    fds[1].fd = err.get_fh().get();
    fds[1].events = POLLIN;

    do {
        fds[0].revents = 0;
        fds[1].revents = 0;
        if (::poll(fds, 2, -1) == -1)
            break;

        if (fds[0].revents & POLLIN) {
            std::string line;
            if (atf::io::getline(out, line).good()) {
                if (line == "__atf_stream_end__")
                    fds[0].events &= ~POLLIN;
                else
                    got_stdout_line(line);
            } else
                fds[0].events &= ~POLLIN;
        }

        if (fds[1].revents & POLLIN) {
            std::string line;
            if (atf::io::getline(err, line).good()) {
                if (line == "__atf_stream_end__")
                    fds[1].events &= ~POLLIN;
                else
                    got_stderr_line(line);
            } else
                fds[1].events &= ~POLLIN;
        }
    } while (fds[0].events & POLLIN || fds[1].events & POLLIN);

    if ((!out.good() || !err.good()))
        throw serial::format_error("Missing terminators in stdout or stderr");
}

void
impl::atf_tcs_reader::read(atf::io::unbuffered_istream& out,
                           atf::io::unbuffered_istream& err)
{
    std::string line;

    size_t ntcs = read_size_t(m_int, "Failed to read number of test cases");
    got_ntcs(ntcs);

    std::string tcname;
    for (size_t i = 0; i < ntcs; i++) {
        m_int.getline(line);
        tcname = line;
        got_tc_start(tcname);

        read_out_err(out, err);

        tests::tcr tcr = read_tcr(m_int);
        read_terminator(m_int, "Missing test case terminator");
        got_tc_end(tcr);

        tcname.clear();
    }

    got_eof();
}

// ------------------------------------------------------------------------
// The "atf_tcs_writer" class.
// ------------------------------------------------------------------------

impl::atf_tcs_writer::atf_tcs_writer(std::ostream& os, size_t ntcs) :
    m_ext(os, "application/X-atf-tcs", 0)
{
    m_ext.putline(ntcs);
    m_ext.flush();
}

void
impl::atf_tcs_writer::start_tc(const std::string& tcname)
{
    m_ext.putline(tcname);
    m_ext.flush();
}

void
impl::atf_tcs_writer::end_tc(const atf::tests::tcr& tcr)
{
    std::cout << "__atf_stream_end__\n";
    std::cout.flush();
    std::cerr << "__atf_stream_end__\n";
    std::cerr.flush();
    switch (tcr.get_status()) {
    case tests::tcr::status_passed:
        m_ext.putline("passed");
        break;

    case tests::tcr::status_failed:
        m_ext.putline("failed");
        m_ext.putline(tcr.get_reason());
        break;

    case tests::tcr::status_skipped:
        m_ext.putline("skipped");
        m_ext.putline(tcr.get_reason());
        break;

    default:
        assert(false);
    }
    m_ext.putline(".");
    m_ext.flush();
}

// ------------------------------------------------------------------------
// The "atf_tps_reader" class.
// ------------------------------------------------------------------------

impl::atf_tps_reader::atf_tps_reader(std::istream& is) :
    m_int(is, "application/X-atf-tps", 0)
{
}

impl::atf_tps_reader::~atf_tps_reader(void)
{
}

void
impl::atf_tps_reader::got_ntps(size_t ntps)
{
}

void
impl::atf_tps_reader::got_tp_start(const atf::fs::path& tp, size_t ntcs)
{
}

void
impl::atf_tps_reader::got_tp_end(void)
{
}

void
impl::atf_tps_reader::got_tc_start(const std::string& tcname)
{
}

void
impl::atf_tps_reader::got_tc_stdout_line(const std::string& line)
{
}

void
impl::atf_tps_reader::got_tc_stderr_line(const std::string& line)
{
}

void
impl::atf_tps_reader::got_tc_end(const atf::tests::tcr& tcr)
{
}

void
impl::atf_tps_reader::got_eof(void)
{
}

void
impl::atf_tps_reader::read(void)
{
    std::string line;

    size_t ntps = read_size_t(m_int, "Failed to read number of test programs");
    got_ntps(ntps);

    while (m_int.getline(line).good()) {
        atf::fs::path tp(line);

        m_int.getline(line);
        size_t ntcs;
        {
            std::istringstream ss;
            ss.str(line);
            ss >> ntcs;
        }

        got_tp_start(tp, ntcs);

        for (size_t i = 0; i < ntcs; i++) {
            m_int.getline(line);
            got_tc_start(line);

            bool done = false;
            while (!done && m_int.getline(line).good()) {
                if (line.substr(0, 4) == "so: ") {
                    got_tc_stdout_line(line.substr(5));
                } else if (line.substr(0, 4) == "se: ") {
                    got_tc_stderr_line(line.substr(5));
                } else {
                    tests::tcr tcr = read_tcr(m_int, line);
                    read_terminator(m_int, "Missing test case terminator");
                    got_tc_end(tcr);

                    done = true;
                }
            }
        }

        read_terminator(m_int, "Missing test program terminator");

        got_tp_end();
    }

    got_eof();
}

// ------------------------------------------------------------------------
// The "atf_tps_writer" class.
// ------------------------------------------------------------------------

impl::atf_tps_writer::atf_tps_writer(std::ostream& os, size_t ntps) :
    m_ext(os, "application/X-atf-tps", 0)
{
    m_ext.putline(ntps);
    m_ext.flush();
}

void
impl::atf_tps_writer::start_tp(const atf::fs::path& tp, size_t ntcs)
{
    m_ext.putline(tp.str());
    m_ext.putline(ntcs);
    m_ext.flush();
}

void
impl::atf_tps_writer::end_tp(void)
{
    m_ext.putline(".");
    m_ext.flush();
}

void
impl::atf_tps_writer::start_tc(const std::string& tcname)
{
    m_ext.putline(tcname);
    m_ext.flush();
}

void
impl::atf_tps_writer::stdout_tc(const std::string& line)
{
    m_ext.putline("so: " + line);
    m_ext.flush();
}

void
impl::atf_tps_writer::stderr_tc(const std::string& line)
{
    m_ext.putline("se: " + line);
    m_ext.flush();
}

void
impl::atf_tps_writer::end_tc(const atf::tests::tcr& tcr)
{
    switch (tcr.get_status()) {
    case atf::tests::tcr::status_passed:
        m_ext.putline("passed");
        break;

    case atf::tests::tcr::status_skipped:
        m_ext.putline("skipped");
        m_ext.putline(tcr.get_reason());
        break;

    case atf::tests::tcr::status_failed:
        m_ext.putline("failed");
        m_ext.putline(tcr.get_reason());
        break;

    default:
        assert(false);
    }
    m_ext.putline(".");
    m_ext.flush();
}
