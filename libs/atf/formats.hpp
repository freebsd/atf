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

#if !defined(_ATF_FORMATS_HPP_)
#define _ATF_FORMATS_HPP_

#include <string>

#include <atf/fs.hpp>
#include <atf/io.hpp>
#include <atf/serial.hpp>
#include <atf/tests.hpp>

namespace atf {
namespace formats {

// ------------------------------------------------------------------------
// The "atf_atffile_reader" class.
// ------------------------------------------------------------------------

class atf_atffile_reader {
    std::istream& m_is;
    atf::serial::internalizer m_int;

protected:
    virtual void got_tp(const std::string&);
    virtual void got_var(const std::string&, const std::string &);
    virtual void got_eof(void);

public:
    atf_atffile_reader(std::istream&);
    virtual ~atf_atffile_reader(void);

    void read(void);
};

// ------------------------------------------------------------------------
// The "atf_config_reader" class.
// ------------------------------------------------------------------------

class atf_config_reader {
    std::istream& m_is;
    atf::serial::internalizer m_int;

protected:
    virtual void got_var(const std::string&, const std::string &);
    virtual void got_eof(void);

public:
    atf_config_reader(std::istream&);
    virtual ~atf_config_reader(void);

    void read(void);
};

// ------------------------------------------------------------------------
// The "atf_tcs_reader" class.
// ------------------------------------------------------------------------

class atf_tcs_reader {
    atf::io::pistream& m_is;
    atf::serial::internalizer m_int;

protected:
    virtual void got_ntcs(size_t);
    virtual void got_tc_start(const std::string&);
    virtual void got_tc_end(const atf::tests::tcr&);
    virtual void got_stdout_line(const std::string&);
    virtual void got_stderr_line(const std::string&);
    virtual void got_eof(void);

public:
    atf_tcs_reader(atf::io::pistream&);
    virtual ~atf_tcs_reader(void);

    void read(atf::io::pistream&, atf::io::pistream&);
};

// ------------------------------------------------------------------------
// The "atf_tcs_writer" class.
// ------------------------------------------------------------------------

class atf_tcs_writer {
    atf::serial::externalizer m_ext;

public:
    atf_tcs_writer(std::ostream&, size_t);

    void start_tc(const std::string&);
    void end_tc(const atf::tests::tcr&);
};

// ------------------------------------------------------------------------
// The "atf_tps_reader" class.
// ------------------------------------------------------------------------

class atf_tps_reader {
    std::istream& m_is;
    atf::serial::internalizer m_int;

protected:
    virtual void got_ntps(size_t);
    virtual void got_tp_start(const atf::fs::path&, size_t);
    virtual void got_tp_end(void);

    virtual void got_tc_start(const std::string&);
    virtual void got_tc_stdout_line(const std::string&);
    virtual void got_tc_stderr_line(const std::string&);
    virtual void got_tc_end(const atf::tests::tcr&);
    virtual void got_eof(void);

public:
    atf_tps_reader(std::istream&);
    virtual ~atf_tps_reader(void);

    void read(void);
};

// ------------------------------------------------------------------------
// The "atf_tps_writer" class.
// ------------------------------------------------------------------------

class atf_tps_writer {
    atf::serial::externalizer m_ext;

public:
    atf_tps_writer(std::ostream&, size_t);

    void start_tp(const atf::fs::path&, size_t);
    void end_tp(void);

    void start_tc(const std::string&);
    void stdout_tc(const std::string&);
    void stderr_tc(const std::string&);
    void end_tc(const atf::tests::tcr&);
};

} // namespace formats
} // namespace atf

#endif // !defined(_ATF_FORMATS_HPP_)
