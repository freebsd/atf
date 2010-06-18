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

bin_PROGRAMS += atf-report/atf-report
atf_report_atf_report_SOURCES = atf-report/atf-report.cpp \
                                atf-report/reader.cpp \
                                atf-report/reader.hpp
atf_report_atf_report_LDADD = libatf-c++.la
dist_man_MANS += atf-report/atf-report.1

cssdir = $(atf_cssdir)
css_DATA = atf-report/tests-results.css
EXTRA_DIST += $(css_DATA)

dtddir = $(atf_dtddir)
dtd_DATA = atf-report/tests-results.dtd
EXTRA_DIST += $(dtd_DATA)

xsldir = $(atf_xsldir)
xsl_DATA = atf-report/tests-results.xsl
EXTRA_DIST += $(xsl_DATA)

tests_atf_report_DATA = atf-report/Atffile
tests_atf_reportdir = $(pkgtestsdir)/atf-report
EXTRA_DIST += $(tests_atf_report_DATA)

CXX_TP([atf-report], [h_fail])
CXX_TP([atf-report], [h_pass])
CXX_TP([atf-report], [h_misc])
SH_TP([atf-report], [integration_test])
CXX_TP([atf-report], [reader_test], [atf-report/reader.cpp],
       [atf-c++/libh.la], [-I$(srcdir)/atf-report -I$(srcdir)/atf-c++])

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
