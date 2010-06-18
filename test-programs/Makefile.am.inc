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

tests_test_programs_DATA = test-programs/Atffile
tests_test_programsdir = $(pkgtestsdir)/test-programs
EXTRA_DIST += $(tests_test_programs_DATA)

tests_test_programs_PROGRAMS = test-programs/h_c
test_programs_h_c_SOURCES = test-programs/h_c.c
test_programs_h_c_LDADD = libatf-c.la

tests_test_programs_PROGRAMS += test-programs/h_cpp
test_programs_h_cpp_SOURCES = test-programs/h_cpp.cpp
test_programs_h_cpp_LDADD = libatf-c++.la

common_sh = $(srcdir)/test-programs/common.sh
EXTRA_DIST += test-programs/common.sh

tests_test_programs_SCRIPTS = test-programs/h_sh
CLEANFILES += test-programs/h_sh
EXTRA_DIST += test-programs/h_sh.sh
test-programs/h_sh: $(srcdir)/test-programs/h_sh.sh
	test -d test-programs || mkdir -p test-programs
	@src="$(srcdir)/test-programs/h_sh.sh $(common_sh)"; \
	dst="test-programs/h_sh"; $(BUILD_SH_TP)

tests_test_programs_SCRIPTS += test-programs/config_test
CLEANFILES += test-programs/config_test
EXTRA_DIST += test-programs/config_test.sh
test-programs/config_test: $(srcdir)/test-programs/config_test.sh
	test -d test-programs || mkdir -p test-programs
	@src="$(srcdir)/test-programs/config_test.sh $(common_sh)"; \
	dst="test-programs/config_test"; $(BUILD_SH_TP)

tests_test_programs_SCRIPTS += test-programs/fork_test
CLEANFILES += test-programs/fork_test
EXTRA_DIST += test-programs/fork_test.sh
test-programs/fork_test: $(srcdir)/test-programs/fork_test.sh
	test -d test-programs || mkdir -p test-programs
	@src="$(srcdir)/test-programs/fork_test.sh $(common_sh)"; \
	dst="test-programs/fork_test"; $(BUILD_SH_TP)

tests_test_programs_SCRIPTS += test-programs/meta_data_test
CLEANFILES += test-programs/meta_data_test
EXTRA_DIST += test-programs/meta_data_test.sh
test-programs/meta_data_test: $(srcdir)/test-programs/meta_data_test.sh
	test -d test-programs || mkdir -p test-programs
	@src="$(srcdir)/test-programs/meta_data_test.sh $(common_sh)"; \
	dst="test-programs/meta_data_test"; $(BUILD_SH_TP)

tests_test_programs_SCRIPTS += test-programs/result_test
CLEANFILES += test-programs/result_test
EXTRA_DIST += test-programs/result_test.sh
test-programs/result_test: $(srcdir)/test-programs/result_test.sh
	test -d test-programs || mkdir -p test-programs
	@src="$(srcdir)/test-programs/result_test.sh $(common_sh)"; \
	dst="test-programs/result_test"; $(BUILD_SH_TP)

tests_test_programs_SCRIPTS += test-programs/srcdir_test
CLEANFILES += test-programs/srcdir_test
EXTRA_DIST += test-programs/srcdir_test.sh
test-programs/srcdir_test: $(srcdir)/test-programs/srcdir_test.sh
	test -d test-programs || mkdir -p test-programs
	@src="$(srcdir)/test-programs/srcdir_test.sh $(common_sh)"; \
	dst="test-programs/srcdir_test"; $(BUILD_SH_TP)

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
