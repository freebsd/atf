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

#define CALLBACK(parser, func) \
    do { \
        if (!(parser).has_errors()) \
            func; \
    } while (false);

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
// The "atf_atffile" auxiliary parser.
// ------------------------------------------------------------------------

namespace atf_atffile {

enum tokens {
    eof,
    nl,
    text,
    dblquote,
    equal,
    colon,
    hash,
    conf,
    prop,
    tp,
    tp_glob,
};

typedef atf::parser::token< tokens > token;

class tokenizer : public atf::parser::tokenizer
    < tokens, atf::serial::internalizer > {
public:
    tokenizer(atf::serial::internalizer& is) :
        atf::parser::tokenizer< tokens, atf::serial::internalizer >
            (is, true, eof, nl, text, is.lineno())
    {
        add_delim(':', colon);
        add_delim('=', equal);
        add_delim('#', hash);
        add_quote('"', dblquote);
        add_keyword("conf", conf);
        add_keyword("prop", prop);
        add_keyword("tp", tp);
        add_keyword("tp-glob", tp_glob);
    }
};

} // namespace atf_atffile

// ------------------------------------------------------------------------
// The "atf_config" auxiliary parser.
// ------------------------------------------------------------------------

namespace atf_config {

enum tokens {
    eof,
    nl,
    text,
    dblquote,
    equal,
    hash,
};

typedef atf::parser::token< tokens > token;

class tokenizer : public atf::parser::tokenizer
    < tokens, atf::serial::internalizer > {
public:
    tokenizer(atf::serial::internalizer& is) :
        atf::parser::tokenizer< tokens, atf::serial::internalizer >
            (is, true, eof, nl, text, is.lineno())
    {
        add_delim('=', equal);
        add_delim('#', hash);
        add_quote('"', dblquote);
    }
};

} // namespace atf_config

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
            (is, true, eof, nl, text, is.lineno())
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
            (is, true, eof, nl, text, is.lineno())
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

} // namespace atf_tps

// ------------------------------------------------------------------------
// The "atf_atffile_reader" class.
// ------------------------------------------------------------------------

impl::atf_atffile_reader::atf_atffile_reader(std::istream& is) :
    m_int(is, "application/X-atf-atffile", 1)
{
}

impl::atf_atffile_reader::~atf_atffile_reader(void)
{
}

void
impl::atf_atffile_reader::got_conf(const std::string& name,
                                   const std::string& val)
{
}

void
impl::atf_atffile_reader::got_prop(const std::string& name,
                                   const std::string& val)
{
}

void
impl::atf_atffile_reader::got_tp(const std::string& name, bool isglob)
{
}

void
impl::atf_atffile_reader::got_eof(void)
{
}

void
impl::atf_atffile_reader::read(void)
{
    using atf::parser::parse_error;

    atf_atffile::tokenizer tkz(m_int);
    atf::parser::parser< atf_atffile::tokenizer > p(tkz);

    for (;;) {
        try {
            atf_atffile::token t =
                p.expect(atf_atffile::conf, atf_atffile::hash,
                         atf_atffile::prop, atf_atffile::tp,
                         atf_atffile::tp_glob, atf_atffile::nl,
                         atf_atffile::eof,
                         "conf, #, prop, tp, tp-glob, a new line or eof");
            if (t.type() == atf_atffile::eof)
                break;

            if (t.type() == atf_atffile::conf) {
                t = p.expect(atf_atffile::colon, "`:'");

                t = p.expect(atf_atffile::text, "variable name");
                std::string var = t.text();

                t = p.expect(atf_atffile::equal, "equal sign");

                t = p.expect(atf_atffile::text, "word or quoted string");
                CALLBACK(p, got_conf(var, t.text()));
            } else if (t.type() == atf_atffile::hash) {
                (void)p.rest_of_line();
            } else if (t.type() == atf_atffile::prop) {
                t = p.expect(atf_atffile::colon, "`:'");

                t = p.expect(atf_atffile::text, "property name");
                std::string name = t.text();

                t = p.expect(atf_atffile::equal, "equal sign");

                t = p.expect(atf_atffile::text, "word or quoted string");
                CALLBACK(p, got_prop(name, t.text()));
            } else if (t.type() == atf_atffile::tp) {
                t = p.expect(atf_atffile::colon, "`:'");

                t = p.expect(atf_atffile::text, "word or quoted string");
                CALLBACK(p, got_tp(t.text(), false));
            } else if (t.type() == atf_atffile::tp_glob) {
                t = p.expect(atf_atffile::colon, "`:'");

                t = p.expect(atf_atffile::text, "word or quoted string");
                CALLBACK(p, got_tp(t.text(), true));
            } else if (t.type() == atf_atffile::nl) {
                continue;
            } else
                assert(false);

            t = p.expect(atf_atffile::nl, atf_atffile::hash,
                         "new line or comment");
            if (t.type() == atf_atffile::hash) {
                (void)p.rest_of_line();
                t = p.next();
            }
        } catch (const parse_error& pe) {
            p.add_error(pe);
            p.reset(atf_atffile::nl);
        }
    }

    CALLBACK(p, got_eof());
}

// ------------------------------------------------------------------------
// The "atf_config_reader" class.
// ------------------------------------------------------------------------

impl::atf_config_reader::atf_config_reader(std::istream& is) :
    m_int(is, "application/X-atf-config", 1)
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
    using atf::parser::parse_error;

    atf_config::tokenizer tkz(m_int);
    atf::parser::parser< atf_config::tokenizer > p(tkz);

    atf_config::token t = p.next();
    while (t.type() != atf_config::eof) {
        try {
            if (t.type() == atf_config::hash) {
                (void)p.rest_of_line();
            } else if (t.type() == atf_config::text) {
                std::string name = t.text();

                t = p.expect(atf_config::equal, "equal sign");

                t = p.expect(atf_config::text, "word or quoted string");
                CALLBACK(p, got_var(name, t.text()));
            } else if (t.type() == atf_config::nl) {
                t = p.next();
                continue;
            } else {
                throw parse_error(t.lineno(), "Unexpected token `" +
                                  t.text() + "'");
            }

            t = p.expect(atf_config::nl, atf_config::hash,
                         "new line or comment");
            if (t.type() == atf_config::hash) {
                (void)p.rest_of_line();
                t = p.next();
            }
        } catch (const parse_error& pe) {
            p.add_error(pe);
            t = p.reset(atf_config::nl);
        }

        t = p.next();
    }
    //p.expect(atf_config::eof, "end of stream");

    CALLBACK(p, got_eof());
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
impl::atf_tcs_reader::read_out_err(void* pptr,
                                   atf::io::unbuffered_istream& out,
                                   atf::io::unbuffered_istream& err)
{
    atf::parser::parser< atf_tps::tokenizer >& p =
        *reinterpret_cast< atf::parser::parser< atf_tps::tokenizer >* >
        (pptr);

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
                    CALLBACK(p, got_stdout_line(line));
            } else
                fds[0].events &= ~POLLIN;
        }

        if (fds[1].revents & POLLIN) {
            std::string line;
            if (atf::io::getline(err, line).good()) {
                if (line == "__atf_tc_separator__")
                    fds[1].events &= ~POLLIN;
                else
                    CALLBACK(p, got_stderr_line(line));
            } else
                fds[1].events &= ~POLLIN;
        }
    } while (fds[0].events & POLLIN || fds[1].events & POLLIN);
}

void
impl::atf_tcs_reader::read(atf::io::unbuffered_istream& out,
                           atf::io::unbuffered_istream& err)
{
    using atf::parser::parse_error;

    atf_tcs::tokenizer tkz(m_int);
    atf::parser::parser< atf_tcs::tokenizer > p(tkz);

    try {
        atf_tcs::token t = p.expect(atf_tcs::tcs_count, "tcs-count field");
        t = p.expect(atf_tcs::colon, "`:'");

        t = p.expect(atf_tcs::text, "number of test cases");
        size_t ntcs = string_to_size_t(t.text());
        CALLBACK(p, got_ntcs(ntcs));

        t = p.expect(atf_tcs::nl, "new line");

        size_t i = 0;
        while (m_int.good() && i < ntcs) {
            try {
                t = p.expect(atf_tcs::tc_start, "start of test case");

                t = p.expect(atf_tcs::colon, "`:'");

                t = p.expect(atf_tcs::text, "test case name");
                std::string tcname = t.text();
                CALLBACK(p, got_tc_start(tcname));

                t = p.expect(atf_tcs::nl, "new line");

                read_out_err(&p, out, err);
                if (i < ntcs - 1 && (!out.good() || !err.good()))
                    p.add_error(parse_error(0, "Missing terminators in "
                                               "stdout or stderr"));

                t = p.expect(atf_tcs::tc_end, "end of test case");

                t = p.expect(atf_tcs::colon, "`:'");

                t = p.expect(atf_tcs::text, "test case name");
                if (t.text() != tcname)
                    throw parse_error(t.lineno(), "Test case name used in "
                                                  "terminator does not match "
                                                  "opening");

                t = p.expect(atf_tcs::comma, "`,'");

                t = p.expect(atf_tcs::passed, atf_tcs::skipped,
                             atf_tcs::failed, "passed, failed or skipped");
                switch (t.type()) {
                case atf_tcs::passed:
                    CALLBACK(p, got_tc_end(tests::tcr::passed()));
                    break;

                case atf_tcs::failed:
                    {
                        t = p.expect(atf_tcs::comma, "`,'");
                        std::string reason = text::trim(p.rest_of_line());
                        if (reason.empty())
                            throw parse_error(t.lineno(),
                                              "Empty reason for failed "
                                              "test case result");
                        CALLBACK(p, got_tc_end(tests::tcr::failed(reason)));
                    }
                    break;

                case atf_tcs::skipped:
                    {
                        t = p.expect(atf_tcs::comma, "`,'");
                        std::string reason = text::trim(p.rest_of_line());
                        if (reason.empty())
                            throw parse_error(t.lineno(),
                                              "Empty reason for skipped "
                                              "test case result");
                        CALLBACK(p, got_tc_end(tests::tcr::skipped(reason)));
                    }
                    break;

                default:
                    assert(false);
                }

                t = p.expect(atf_tcs::nl, "new line");
                i++;
            } catch (const parse_error& pe) {
                p.add_error(pe);
                p.reset(atf_tcs::nl);
            }
        }

        t = p.expect(atf_tcs::eof, "end of stream");
        CALLBACK(p, got_eof());
    } catch (const parse_error& pe) {
        p.add_error(pe);
        p.reset(atf_tcs::nl);
    }
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
impl::atf_tps_reader::read_tp(void* pptr)
{
    using atf::parser::parse_error;

    atf::parser::parser< atf_tps::tokenizer >& p =
        *reinterpret_cast< atf::parser::parser< atf_tps::tokenizer >* >
        (pptr);

    atf_tps::token t = p.expect(atf_tps::tp_start, "start of test program");

    t = p.expect(atf_tps::colon, "`:'");

    t = p.expect(atf_tps::text, "test program name");
    std::string tpname = t.text();

    t = p.expect(atf_tps::comma, "`,'");

    t = p.expect(atf_tps::text, "number of test programs");
    size_t ntcs = string_to_size_t(t.text());

    t = p.expect(atf_tps::nl, "new line");

    CALLBACK(p, got_tp_start(tpname, ntcs));

    size_t i = 0;
    while (p.good() && i < ntcs) {
        try {
            read_tc(&p);
            i++;
        } catch (const parse_error& pe) {
            p.add_error(pe);
            p.reset(atf_tps::nl);
        }
    }
    t = p.expect(atf_tps::tp_end, "end of test program");

    t = p.expect(atf_tps::colon, "`:'");

    t = p.expect(atf_tps::text, "test program name");
    if (t.text() != tpname)
        throw parse_error(t.lineno(), "Test program name used in "
                                      "terminator does not match "
                                      "opening");

    t = p.expect(atf_tps::nl, atf_tps::comma, "new line or comma");
    std::string reason;
    if (t.type() == atf_tps::comma) {
        reason = text::trim(p.rest_of_line());
        if (reason.empty())
            throw parse_error(t.lineno(),
                              "Empty reason for failed test program");
        t = p.next();
    }

    CALLBACK(p, got_tp_end(reason));
}

void
impl::atf_tps_reader::read_tc(void* pptr)
{
    using atf::parser::parse_error;

    atf::parser::parser< atf_tps::tokenizer >& p =
        *reinterpret_cast< atf::parser::parser< atf_tps::tokenizer >* >
        (pptr);

    atf_tps::token t = p.expect(atf_tps::tc_start, "start of test case");

    t = p.expect(atf_tps::colon, "`:'");

    t = p.expect(atf_tps::text, "test case name");
    std::string tcname = t.text();
    CALLBACK(p, got_tc_start(tcname));

    t = p.expect(atf_tps::nl, "new line");

    t = p.expect(atf_tps::tc_end, atf_tps::tc_so, atf_tps::tc_se,
                 "end of test case or test case's stdout/stderr line");
    while (t.type() != atf_tps::tc_end &&
           (t.type() == atf_tps::tc_so || t.type() == atf_tps::tc_se)) {
        atf_tps::token t2 = t;

        t = p.expect(atf_tps::colon, "`:'");

        std::string line = text::trim(p.rest_of_line());

        if (t2.type() == atf_tps::tc_so) {
            CALLBACK(p, got_tc_stdout_line(line));
        } else {
            assert(t2.type() == atf_tps::tc_se);
            CALLBACK(p, got_tc_stderr_line(line));
        }

        t = p.expect(atf_tps::nl, "new line");

        t = p.expect(atf_tps::tc_end, atf_tps::tc_so, atf_tps::tc_se,
                     "end of test case or test case's stdout/stderr line");
    }

    t = p.expect(atf_tps::colon, "`:'");

    t = p.expect(atf_tps::text, "test case name");
    if (t.text() != tcname)
        throw parse_error(t.lineno(),
                          "Test case name used in terminator does not "
                          "match opening");

    t = p.expect(atf_tps::comma, "`,'");

    t = p.expect(atf_tps::passed, atf_tps::failed, atf_tps::skipped,
                 "passed, failed or skipped");
    switch (t.type()) {
    case atf_tps::passed:
        CALLBACK(p, got_tc_end(tests::tcr::passed()));
        break;

    case atf_tps::failed:
        {
            t = p.expect(atf_tps::comma, "`,'");
            std::string reason = text::trim(p.rest_of_line());
            if (reason.empty())
                throw parse_error(t.lineno(),
                                  "Empty reason for failed test case result");
            CALLBACK(p, got_tc_end(tests::tcr::failed(reason)));
        }
        break;

    case atf_tps::skipped:
        {
            t = p.expect(atf_tps::comma, "`,'");
            std::string reason = text::trim(p.rest_of_line());
            if (reason.empty())
                throw parse_error(t.lineno(),
                                  "Empty reason for skipped test case result");
            CALLBACK(p, got_tc_end(tests::tcr::skipped(reason)));
        }
        break;

    default:
        assert(false);
    }

    t = p.expect(atf_tps::nl, "new line");
}

void
impl::atf_tps_reader::read(void)
{
    using atf::parser::parse_error;

    atf_tps::tokenizer tkz(m_int);
    atf::parser::parser< atf_tps::tokenizer > p(tkz);

    try {
        atf_tps::token t = p.expect(atf_tps::tps_count, "tps-count field");

        t = p.expect(atf_tps::colon, "`:'");

        t = p.expect(atf_tps::text, "number of test programs");
        size_t ntps = string_to_size_t(t.text());
        CALLBACK(p, got_ntps(ntps));

        t = p.expect(atf_tps::nl, "new line");

        size_t i = 0;
        while (p.good() && i < ntps) {
            try {
                read_tp(&p);
                i++;
            } catch (const parse_error& pe) {
                p.add_error(pe);
                p.reset(atf_tps::nl);
            }
        }

        t = p.expect(atf_tps::eof, "end of stream");
        CALLBACK(p, got_eof());
    } catch (const parse_error& pe) {
        p.add_error(pe);
        p.reset(atf_tps::nl);
    }
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
