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

EXTRA_DIST += test-programs/common.sh

C_TP([test-programs], [h_c], [], [atf-c/libh.la])
CXX_TP([test-programs], [h_cpp], [], [atf-c++/libh.la])
SH_TP([test-programs], [h_sh])
SH_TP([test-programs], [config_test], [$(srcdir)/test-programs/common.sh])
SH_TP([test-programs], [fork_test], [$(srcdir)/test-programs/common.sh])
SH_TP([test-programs], [meta_data_test], [$(srcdir)/test-programs/common.sh])
SH_TP([test-programs], [result_test], [$(srcdir)/test-programs/common.sh])
SH_TP([test-programs], [srcdir_test], [$(srcdir)/test-programs/common.sh])

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
