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

bin_PROGRAMS += atf-check/atf-check
atf_check_atf_check_SOURCES = atf-check/atf-check.cpp
atf_check_atf_check_LDADD = libatf-c++.la
dist_man_MANS += atf-check/atf-check.1

tests_atf_check_DATA = atf-check/Atffile
tests_atf_checkdir = $(pkgtestsdir)/atf-check
EXTRA_DIST += $(tests_atf_check_DATA)

tests_atf_check_SCRIPTS = atf-check/integration_test
CLEANFILES += atf-check/integration_test
EXTRA_DIST += atf-check/integration_test.sh
atf-check/integration_test: $(srcdir)/atf-check/integration_test.sh
	test -d atf-check || mkdir -p atf-check
	@src="$(srcdir)/atf-check/integration_test.sh"; \
	dst="atf-check/integration_test"; $(BUILD_SH_TP)

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
