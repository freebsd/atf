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
#include "atf/parser.hpp"
#include "atf/text.hpp"

namespace impl = atf::formats;
#define IMPL_NAME "atf::formats"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

static
size_t
string_to_size_t(const std::string& str)
{
    std::istringstream ss(str);
    size_t s;
    ss >> s;

    return s;
}

// ------------------------------------------------------------------------
// The "atf_tcs" auxiliary parser.
// ------------------------------------------------------------------------

namespace atf_tcs {

enum tokens {
    eof,
    nl,
    text,
    colon,
    comma,
    tcs_count,
    tc_start,
    tc_end,
    passed,
    failed,
    skipped,
};

typedef atf::parser::token< tokens > token;

class tokenizer : public atf::parser::tokenizer
    < tokens, atf::serial::internalizer > {
public:
    tokenizer(atf::serial::internalizer& is) :
        atf::parser::tokenizer< tokens, atf::serial::internalizer >
            (is, true, eof, nl, text)
    {
        add_delim(':', colon);
        add_delim(',', comma);
        add_keyword("tcs-count", tcs_count);
        add_keyword("tc-start", tc_start);
        add_keyword("tc-end", tc_end);
        add_keyword("passed", passed);
        add_keyword("failed", failed);
        add_keyword("skipped", skipped);
    }
};

inline
void
expect(token t, tokens type, const std::string& textual)
{
    if (t.type() != type)
        throw atf::serial::format_error("Unexpected token `" + t.text() +
                                        "'; expected " + textual);
}

} // namespace atf_tcs

// ------------------------------------------------------------------------
// The "atf_tps" auxiliary parser.
// ------------------------------------------------------------------------

namespace atf_tps {

enum tokens {
    eof,
    nl,
    text,
    colon,
    comma,
    tps_count,
    tp_start,
    tp_end,
    tc_start,
    tc_so,
    tc_se,
    tc_end,
    passed,
    failed,
    skipped,
};

typedef atf::parser::token< tokens > token;

class tokenizer : public atf::parser::tokenizer
    < tokens, atf::serial::internalizer > {
public:
    tokenizer(atf::serial::internalizer& is) :
        atf::parser::tokenizer< tokens, atf::serial::internalizer >
            (is, true, eof, nl, text)
    {
        add_delim(':', colon);
        add_delim(',', comma);
        add_keyword("tps-count", tps_count);
        add_keyword("tp-start", tp_start);
        add_keyword("tp-end", tp_end);
        add_keyword("tc-start", tc_start);
        add_keyword("tc-so", tc_so);
        add_keyword("tc-se", tc_se);
        add_keyword("tc-end", tc_end);
        add_keyword("passed", passed);
        add_keyword("failed", failed);
        add_keyword("skipped", skipped);
    }
};

inline
void
expect(token t, tokens type, const std::string& textual)
{
    if (t.type() != type)
        throw atf::serial::format_error("Unexpected token `" + t.text() +
                                        "'; expected " + textual);
}

} // namespace atf_tcs

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
    m_int(is, "application/X-atf-tcs", 1)
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
                if (line == "__atf_tc_separator__")
                    fds[0].events &= ~POLLIN;
                else
                    got_stdout_line(line);
            } else
                fds[0].events &= ~POLLIN;
        }

        if (fds[1].revents & POLLIN) {
            std::string line;
            if (atf::io::getline(err, line).good()) {
                if (line == "__atf_tc_separator__")
                    fds[1].events &= ~POLLIN;
                else
                    got_stderr_line(line);
            } else
                fds[1].events &= ~POLLIN;
        }
    } while (fds[0].events & POLLIN || fds[1].events & POLLIN);
}

void
impl::atf_tcs_reader::read(atf::io::unbuffered_istream& out,
                           atf::io::unbuffered_istream& err)
{
    atf_tcs::tokenizer tkz(m_int);

    atf_tcs::token t = tkz.next();
    atf_tcs::expect(t, atf_tcs::tcs_count, "tcs-count field");
    t = tkz.next();
    atf_tcs::expect(t, atf_tcs::colon, "`:'");

    t = tkz.next();
    atf_tcs::expect(t, atf_tcs::text, "number of test cases");
    size_t ntcs = string_to_size_t(t.text());
    got_ntcs(ntcs);

    t = tkz.next();
    atf_tcs::expect(t, atf_tcs::nl, "new line");

    for (size_t i = 0; i < ntcs; i++) {
        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::tc_start, "start of test case");

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::colon, "`:'");

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::text, "test case name");
        std::string tcname = t.text();
        got_tc_start(tcname);

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::nl, "new line");

        read_out_err(out, err);
        if (i < ntcs - 1 && (!out.good() || !err.good()))
            throw serial::format_error("Missing terminators in stdout "
                                       "or stderr");

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::tc_end, "end of test case");

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::colon, "`:'");

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::text, "test case name");
        if (t.text() != tcname)
            throw atf::serial::format_error("Test case name used in "
                                            "terminator does not match "
                                            "opening");

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::comma, "`,'");

        t = tkz.next();
        switch (t.type()) {
        case atf_tcs::passed:
            got_tc_end(tests::tcr::passed());
            break;

        case atf_tcs::failed:
            t = tkz.next();
            atf_tcs::expect(t, atf_tcs::comma, "`,'");
            got_tc_end(tests::tcr::failed(text::trim(tkz.rest_of_line())));
            break;

        case atf_tcs::skipped:
            t = tkz.next();
            atf_tcs::expect(t, atf_tcs::comma, "`,'");
            got_tc_end(tests::tcr::skipped(text::trim(tkz.rest_of_line())));
            break;

        default:
            atf_tcs::expect(t, atf_tcs::passed, "passed, failed or skipped");
        }

        t = tkz.next();
        atf_tcs::expect(t, atf_tcs::nl, "new line");
    }

    t = tkz.next();
    atf_tcs::expect(t, atf_tcs::eof, "end of stream");
    got_eof();
}

// ------------------------------------------------------------------------
// The "atf_tcs_writer" class.
// ------------------------------------------------------------------------

impl::atf_tcs_writer::atf_tcs_writer(std::ostream& os, size_t ntcs) :
    m_ext(os, "application/X-atf-tcs", 1),
    m_ntcs(ntcs),
    m_curtc(0)
{
    m_ext.putline("tcs-count: " + text::to_string(ntcs));
    m_ext.flush();
}

void
impl::atf_tcs_writer::start_tc(const std::string& tcname)
{
    m_tcname = tcname;
    m_ext.putline("tc-start: " + tcname);
    m_ext.flush();
}

void
impl::atf_tcs_writer::end_tc(const atf::tests::tcr& tcr)
{
    assert(m_curtc < m_ntcs);
    m_curtc++;
    if (m_curtc < m_ntcs) {
        std::cout << "__atf_tc_separator__\n";
        std::cerr << "__atf_tc_separator__\n";
    }
    std::cout.flush();
    std::cerr.flush();

    std::string end = "tc-end: " + m_tcname + ", ";
    switch (tcr.get_status()) {
    case tests::tcr::status_passed:
        end += "passed";
        break;

    case tests::tcr::status_failed:
        end += "failed, " + tcr.get_reason();
        break;

    case tests::tcr::status_skipped:
        end += "skipped, " + tcr.get_reason();
        break;

    default:
        assert(false);
    }
    m_ext.putline(end);
    m_ext.flush();
}

// ------------------------------------------------------------------------
// The "atf_tps_reader" class.
// ------------------------------------------------------------------------

impl::atf_tps_reader::atf_tps_reader(std::istream& is) :
    m_int(is, "application/X-atf-tps", 1)
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
impl::atf_tps_reader::got_tp_start(const std::string& tp, size_t ntcs)
{
}

void
impl::atf_tps_reader::got_tp_end(const std::string& reason)
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
impl::atf_tps_reader::read_tp(void)
{
    atf_tps::tokenizer tkz(m_int);

    atf_tps::token t = tkz.next();
    atf_tps::expect(t, atf_tps::tp_start, "start of test program");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::colon, "`:'");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::text, "test program name");
    std::string tpname = t.text();

    t = tkz.next();
    atf_tps::expect(t, atf_tps::comma, "`,'");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::text, "number of test programs");
    size_t ntcs = string_to_size_t(t.text());

    t = tkz.next();
    atf_tps::expect(t, atf_tps::nl, "new line");

    got_tp_start(tpname, ntcs);

    for (size_t i = 0; i < ntcs; i++)
        read_tc();

    t = tkz.next();
    atf_tps::expect(t, atf_tps::tp_end, "end of test program");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::colon, "`:'");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::text, "test program name");
    if (t.text() != tpname)
        throw atf::serial::format_error("Test program name used in "
                                        "terminator does not match "
                                        "opening");

    t = tkz.next();
    std::string reason;
    if (t.type() == atf_tps::comma) {
        t = tkz.next();
        reason = text::trim(tkz.rest_of_line());
        t = tkz.next();
    }
    atf_tps::expect(t, atf_tps::nl, "new line");

    got_tp_end(reason);
}

void
impl::atf_tps_reader::read_tc(void)
{
    atf_tps::tokenizer tkz(m_int);

    atf_tps::token t = tkz.next();
    atf_tps::expect(t, atf_tps::tc_start, "start of test case");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::colon, "`:'");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::text, "test case name");
    std::string tcname = t.text();
    got_tc_start(tcname);

    t = tkz.next();
    atf_tps::expect(t, atf_tps::nl, "new line");

    t = tkz.next();
    while (t.type() != atf_tps::tc_end &&
           (t.type() == atf_tps::tc_so || t.type() == atf_tps::tc_se)) {
        atf_tps::token t2 = t;

        t = tkz.next();
        atf_tps::expect(t, atf_tps::colon, "`:'");

        std::string line = text::trim(tkz.rest_of_line());

        if (t2.type() == atf_tps::tc_so) {
            got_tc_stdout_line(line);
        } else {
            assert(t2.type() == atf_tps::tc_se);
            got_tc_stderr_line(line);
        }

        t = tkz.next();
        atf_tps::expect(t, atf_tps::nl, "new line");

        t = tkz.next();
    }
    atf_tps::expect(t, atf_tps::tc_end, "end of test case");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::colon, "`:'");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::text, "test case name");
    if (t.text() != tcname)
        throw atf::serial::format_error("Test case name used in "
                                        "terminator does not match "
                                        "opening");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::comma, "`,'");

    t = tkz.next();
    switch (t.type()) {
    case atf_tps::passed:
        got_tc_end(tests::tcr::passed());
        break;

    case atf_tps::failed:
        t = tkz.next();
        atf_tps::expect(t, atf_tps::comma, "`,'");
        got_tc_end(tests::tcr::failed(text::trim(tkz.rest_of_line())));
        break;

    case atf_tps::skipped:
        t = tkz.next();
        atf_tps::expect(t, atf_tps::comma, "`,'");
        got_tc_end(tests::tcr::skipped(text::trim(tkz.rest_of_line())));
        break;

    default:
        atf_tps::expect(t, atf_tps::passed, "passed, failed or skipped");
    }

    t = tkz.next();
    atf_tps::expect(t, atf_tps::nl, "new line");
}

void
impl::atf_tps_reader::read(void)
{
    atf_tps::tokenizer tkz(m_int);

    atf_tps::token t = tkz.next();
    atf_tps::expect(t, atf_tps::tps_count, "tps-count field");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::colon, "`:'");

    t = tkz.next();
    atf_tps::expect(t, atf_tps::text, "number of test programs");
    size_t ntps = string_to_size_t(t.text());
    got_ntps(ntps);

    t = tkz.next();
    atf_tps::expect(t, atf_tps::nl, "new line");

    for (size_t i = 0; i < ntps; i++)
        read_tp();

    t = tkz.next();
    atf_tps::expect(t, atf_tps::eof, "end of stream");
    got_eof();
}

// ------------------------------------------------------------------------
// The "atf_tps_writer" class.
// ------------------------------------------------------------------------

impl::atf_tps_writer::atf_tps_writer(std::ostream& os, size_t ntps) :
    m_ext(os, "application/X-atf-tps", 1)
{
    m_ext.putline("tps-count: " + text::to_string(ntps));
    m_ext.flush();
}

void
impl::atf_tps_writer::start_tp(const std::string& tp, size_t ntcs)
{
    m_tpname = tp;
    m_ext.putline("tp-start: " + tp + ", " + text::to_string(ntcs));
    m_ext.flush();
}

void
impl::atf_tps_writer::end_tp(const std::string& reason)
{
    if (reason.empty())
        m_ext.putline("tp-end: " + m_tpname);
    else
        m_ext.putline("tp-end: " + m_tpname + ", " + reason);
    m_ext.flush();
}

void
impl::atf_tps_writer::start_tc(const std::string& tcname)
{
    m_tcname = tcname;
    m_ext.putline("tc-start: " + tcname);
    m_ext.flush();
}

void
impl::atf_tps_writer::stdout_tc(const std::string& line)
{
    m_ext.putline("tc-so: " + line);
    m_ext.flush();
}

void
impl::atf_tps_writer::stderr_tc(const std::string& line)
{
    m_ext.putline("tc-se: " + line);
    m_ext.flush();
}

void
impl::atf_tps_writer::end_tc(const atf::tests::tcr& tcr)
{
    std::string str = "tc-end: " + m_tcname + ", ";
    switch (tcr.get_status()) {
    case atf::tests::tcr::status_passed:
        str += "passed";
        break;

    case atf::tests::tcr::status_skipped:
        str += "skipped, " + tcr.get_reason();
        break;

    case atf::tests::tcr::status_failed:
        str += "failed, " + tcr.get_reason();
        break;

    default:
        assert(false);
    }
    m_ext.putline(str);
    m_ext.flush();
}
