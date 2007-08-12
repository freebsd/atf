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
read_size_t(std::istream& is, const std::string& err)
{
    if (!is.good())
        throw atf::serial::format_error(err);

    std::string line;
    std::getline(is, line);

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
read_tcr(std::istream& is, const std::string& firstline)
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

            std::getline(is, line);
            tcr = atf::tests::tcr::failed(line);
            break;

        case atf::tests::tcr::status_skipped:
            if (!is.good())
                throw atf::serial::format_error("Missing reason for "
                                                "skipped test case result");

            std::getline(is, line);
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
read_tcr(std::istream& is)
{
    if (!is.good())
        throw atf::serial::format_error("Missing test case result");

    atf::tests::tcr tcr;
    {
        std::string line;
        std::getline(is, line);
        tcr = read_tcr(is, line);
    }
    return tcr;
}

static
void
read_terminator(std::istream& is, const std::string& err)
{
    if (!is.good())
        throw atf::serial::format_error(err);

    std::string line;
    std::getline(is, line);
    if (line != ".")
        throw atf::serial::format_error(err);
}

// ------------------------------------------------------------------------
// The "atf_tcs_reader" class.
// ------------------------------------------------------------------------

impl::atf_tcs_reader::atf_tcs_reader(atf::io::pistream& is) :
    m_is(is),
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
impl::atf_tcs_reader::read(atf::io::pistream& out, atf::io::pistream& err)
{
    std::string line;

    size_t ntcs = read_size_t(m_is, "Failed to read number of test cases");
    got_ntcs(ntcs);

    struct pollfd fds[3];
    fds[0].fd = m_is.handle().get();
    fds[0].events = POLLIN;
    fds[1].fd = out.handle().get();
    fds[1].events = 0;
    fds[2].fd = err.handle().get();
    fds[2].events = 0;

    std::string tcname;
    do {
        fds[0].revents = 0;
        fds[1].revents = 0;
        fds[2].revents = 0;
        if (::poll(fds, 3, -1) == -1)
            break;

        if (fds[0].revents & POLLIN) {
            if (tcname.empty()) {
                if (std::getline(m_int.get_stream(), line).good()) {
                    tcname = line;
                    got_tc_start(tcname);
                } else
                    fds[0].events &= ~POLLIN;

                if (out.good() || err.good()) {
                    if (out.good())
                        fds[1].events |= POLLIN;
                    if (err.good())
                        fds[2].events |= POLLIN;
                    fds[0].events &= ~POLLIN;
                }
            } else {
                tests::tcr tcr = read_tcr(m_int.get_stream());
                read_terminator(m_int.get_stream(),
                                "Missing test case terminator");
                got_tc_end(tcr);

                tcname.clear();

                if (!m_int.get_stream().good())
                    fds[0].events &= ~POLLIN;

                assert(!(fds[1].events & POLLIN));
                assert(!(fds[2].events & POLLIN));
            }
        }

        if (fds[1].revents & POLLIN) {
            if (std::getline(out, line).good()) {
                if (line == "__atf_stream_end__") {
                    fds[1].events &= ~POLLIN;
                    if (!(fds[2].events & POLLIN))
                        fds[0].events |= POLLIN;
                } else
                    got_stdout_line(line);
            } else
                fds[1].events &= ~POLLIN;
        }

        if (fds[2].revents & POLLIN) {
            if (std::getline(err, line).good()) {
                if (line == "__atf_stream_end__") {
                    fds[2].events &= ~POLLIN;

                    if (!(fds[1].events & POLLIN))
                        fds[0].events |= POLLIN;
                } else
                    got_stderr_line(line);
            } else
                fds[2].events &= ~POLLIN;
        }
    } while (((fds[0].events & POLLIN) && m_int.get_stream().good()) ||
             (((fds[1].events & POLLIN) && out.good()) ||
              ((fds[2].events & POLLIN) && err.good())));

    if (!(fds[0].events & POLLIN) && m_int.get_stream().good()) {
        assert(!tcname.empty());
        throw serial::format_error("Missing terminators in stdout or "
                                   "stderr for test case " + tcname);
    }

    got_eof();
}

// ------------------------------------------------------------------------
// The "atf_tcs_writer" class.
// ------------------------------------------------------------------------

impl::atf_tcs_writer::atf_tcs_writer(std::ostream& os, size_t ntcs) :
    m_ext(os, "application/X-atf-tcs", 0)
{
    m_ext << ntcs << '\n';
    m_ext.flush();
}

void
impl::atf_tcs_writer::start_tc(const std::string& tcname)
{
    m_ext << tcname << '\n';
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
        m_ext << "passed\n";
        break;

    case tests::tcr::status_failed:
        m_ext << "failed\n"
              << tcr.get_reason() << '\n';
        break;

    case tests::tcr::status_skipped:
        m_ext << "skipped\n"
              << tcr.get_reason() << '\n';
        break;

    default:
        assert(false);
    }
    m_ext << ".\n";
    m_ext.flush();
}

// ------------------------------------------------------------------------
// The "atf_tps_reader" class.
// ------------------------------------------------------------------------

impl::atf_tps_reader::atf_tps_reader(std::istream& is) :
    m_is(is),
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

    size_t ntps = read_size_t(m_is, "Failed to read number of test programs");
    got_ntps(ntps);

    while (std::getline(m_is, line).good()) {
        atf::fs::path tp(line);

        std::getline(m_is, line);
        size_t ntcs;
        {
            std::istringstream ss;
            ss.str(line);
            ss >> ntcs;
        }

        got_tp_start(tp, ntcs);

        for (size_t i = 0; i < ntcs; i++) {
            std::getline(m_is, line);
            got_tc_start(line);

            bool done = false;
            while (!done && std::getline(m_is, line).good()) {
                if (line.substr(0, 4) == "so: ") {
                    got_tc_stdout_line(line.substr(5));
                } else if (line.substr(0, 4) == "se: ") {
                    got_tc_stderr_line(line.substr(5));
                } else {
                    tests::tcr tcr = read_tcr(m_is, line);
                    read_terminator(m_int.get_stream(),
                                    "Missing test case terminator");
                    got_tc_end(tcr);

                    done = true;
                }
            }
        }

        read_terminator(m_int.get_stream(),
                        "Missing test program terminator");

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
    m_ext << ntps << '\n';
    m_ext.flush();
}

void
impl::atf_tps_writer::start_tp(const atf::fs::path& tp, size_t ntcs)
{
    m_ext << tp.str() << '\n' << ntcs << '\n';
    m_ext.flush();
}

void
impl::atf_tps_writer::end_tp(void)
{
    m_ext << ".\n";
    m_ext.flush();
}

void
impl::atf_tps_writer::start_tc(const std::string& tcname)
{
    m_ext << tcname << '\n';
    m_ext.flush();
}

void
impl::atf_tps_writer::stdout_tc(const std::string& line)
{
    m_ext << "so: " << line << '\n';
    m_ext.flush();
}

void
impl::atf_tps_writer::stderr_tc(const std::string& line)
{
    m_ext << "se: " << line << '\n';
    m_ext.flush();
}

void
impl::atf_tps_writer::end_tc(const atf::tests::tcr& tcr)
{
    switch (tcr.get_status()) {
    case atf::tests::tcr::status_passed:
        m_ext << "passed" << '\n';
        break;

    case atf::tests::tcr::status_skipped:
        m_ext << "skipped" << '\n' << tcr.get_reason() << '\n';
        break;

    case atf::tests::tcr::status_failed:
        m_ext << "failed" << '\n' << tcr.get_reason() << '\n';
        break;

    default:
        assert(false);
    }
    m_ext << ".\n";
    m_ext.flush();
}
