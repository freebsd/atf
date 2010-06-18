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

bin_PROGRAMS += atf-sh/atf-sh
atf_sh_atf_sh_SOURCES = atf-sh/atf-sh.cpp
atf_sh_atf_sh_LDADD = libatf-c++.la
dist_man_MANS += atf-sh/atf-sh.1

atf_sh_DATA = atf-sh/libatf-sh.subr
atf_shdir = $(pkgdatadir)
EXTRA_DIST += $(atf_sh_DATA)

dist_man_MANS += atf-sh/atf-sh-api.3

tests_atf_sh_DATA = atf-sh/Atffile
tests_atf_shdir = $(pkgtestsdir)/atf-sh
EXTRA_DIST += $(tests_atf_sh_DATA)

SH_TP([atf-sh], [h_misc])
SH_TP([atf-sh], [atf_check_test])
SH_TP([atf-sh], [config_test])
SH_TP([atf-sh], [integration_test])
SH_TP([atf-sh], [normalize_test])
SH_TP([atf-sh], [tc_test])
SH_TP([atf-sh], [tp_test])

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
