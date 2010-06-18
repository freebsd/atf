#
# Automated Testing Framework (atf)
#
# Copyright (c) 2009, 2010 The NetBSD Foundation, Inc.
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

# _AUTOMAKE_ID filename
#
# Converts a filename to an Automake identifier, i.e. a string that can be
# used as part of a variable name.
m4_define([_AUTOMAKE_ID], [m4_translit([$1], [-/.+], [____])])

# _DO_ONCE id what
#
# Do 'what' only once, even if called multiple times.  Uses 'id' to identify
# this specific 'what'.
m4_define([_DO_ONCE], [m4_ifdef([done_$1], [], [$2 m4_define(done_$1, [])])])

# _INIT_VAR var
#
# Initializes a variable to the empty string the first time it is called.
m4_define([_INIT_VAR], [_DO_ONCE($1, [$1 =])])

# C_TP subdir progname extradeps extralibs cppflags
#
# Generates rules to build a C test program.  The and progname is the
# source file name without .c.
m4_define([C_TP], [
_INIT_VAR(_AUTOMAKE_ID(tests_[$1])_PROGRAMS)
_AUTOMAKE_ID(tests_[$1])_PROGRAMS += $1/$2
_AUTOMAKE_ID([$1_$2])_SOURCES = $1/$2.c $3
_AUTOMAKE_ID([$1_$2])_CPPFLAGS = $5
_AUTOMAKE_ID([$1_$2])_LDADD = $4 libatf-c.la
])

# CXX_TP subdir progname extradeps extralibs cppflags
#
# Generates rules to build a C++ test program.  The progname is the
# source file name without .c.
m4_define([CXX_TP], [
_INIT_VAR(_AUTOMAKE_ID(tests_[$1])_PROGRAMS)
_AUTOMAKE_ID(tests_[$1])_PROGRAMS += $1/$2
_AUTOMAKE_ID([$1_$2])_SOURCES = $1/$2.cpp $3
_AUTOMAKE_ID([$1_$2])_CPPFLAGS = $5
_AUTOMAKE_ID([$1_$2])_LDADD = $4 libatf-c++.la
])

# SH_TP subdir progname extradeps
#
# Generates rules to build a shell test program.  The progname is the
# source file name without .c.
m4_define([SH_TP], [
_INIT_VAR(_AUTOMAKE_ID(tests_[$1])_SCRIPTS)
_AUTOMAKE_ID(tests_[$1])_SCRIPTS += $1/$2
CLEANFILES += $1/$2
EXTRA_DIST += $1/$2.sh
$1/$2: $(srcdir)/$1/$2.sh $3
	test -d $1 || mkdir -p $1
	@src="$(srcdir)/$1/$2.sh $3"; dst="$1/$2"; $(BUILD_SH_TP)
])

BUILD_SH_TP = \
	echo "Creating $${dst}"; \
	echo "\#! $(bindir)/atf-sh" >$${dst}; \
	cat $${src} >>$${dst}; \
	chmod +x $${dst}

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
