#
# Automated Testing Framework (atf)
#
# Copyright (c) 2007, 2008, 2009, 2010 The NetBSD Foundation, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
# CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

bin_PROGRAMS += atf-run/atf-run
atf_run_atf_run_SOURCES = atf-run/atf-run.cpp \
                          atf-run/atffile.cpp \
                          atf-run/atffile.hpp \
                          atf-run/config.cpp \
                          atf-run/config.hpp \
                          atf-run/fs.cpp \
                          atf-run/fs.hpp \
                          atf-run/requirements.cpp \
                          atf-run/requirements.hpp \
                          atf-run/test-program.cpp \
                          atf-run/test-program.hpp \
                          atf-run/timer.cpp \
                          atf-run/timer.hpp
atf_run_atf_run_LDADD = libatf-c++.la
dist_man_MANS += atf-run/atf-run.1

tests_atf_run_DATA = atf-run/Atffile
tests_atf_rundir = $(pkgtestsdir)/atf-run
EXTRA_DIST += $(tests_atf_run_DATA)

C_TP([atf-run], [h_bad_metadata])
C_TP([atf-run], [h_several_tcs])
C_TP([atf-run], [h_zero_tcs])
CXX_TP([atf-run], [h_fail])
CXX_TP([atf-run], [h_pass])
CXX_TP([atf-run], [h_misc])
CXX_TP([atf-run], [atffile_test], [atf-run/atffile.cpp],
       [atf-c++/libh.la], [-I$(srcdir)/atf-run -I$(srcdir)/atf-c++])
CXX_TP([atf-run], [config_test], [atf-run/config.cpp],
       [atf-c++/libh.la], [-I$(srcdir)/atf-run -I$(srcdir)/atf-c++])
CXX_TP([atf-run], [fs_test], [atf-run/fs.cpp], [], [-I$(srcdir)/atf-run])
CXX_TP([atf-run], [requirements_test], [atf-run/requirements.cpp], [],
       [-I$(srcdir)/atf-run])
CXX_TP([atf-run], [test_program_test],
       [atf-run/fs.cpp atf-run/test-program.cpp atf-run/timer.cpp],
       [atf-c++/libh.la], [-I$(srcdir)/atf-run -I$(srcdir)/atf-c++])
SH_TP([atf-run], [integration_test])

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
