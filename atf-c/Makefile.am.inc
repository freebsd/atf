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

lib_LTLIBRARIES += libatf-c.la
libatf_c_la_SOURCES = atf-c/build.c \
                      atf-c/build.h \
                      atf-c/check.c \
                      atf-c/check.h \
                      atf-c/config.c \
                      atf-c/config.h \
                      atf-c/dynstr.c \
                      atf-c/dynstr.h \
                      atf-c/env.c \
                      atf-c/env.h \
                      atf-c/error.c \
                      atf-c/error.h \
                      atf-c/error_fwd.h \
                      atf-c/fs.c \
                      atf-c/fs.h \
                      atf-c/list.c \
                      atf-c/list.h \
                      atf-c/macros.h \
                      atf-c/map.c \
                      atf-c/map.h \
                      atf-c/process.c \
                      atf-c/process.h \
                      atf-c/sanity.c \
                      atf-c/sanity.h \
                      atf-c/text.c \
                      atf-c/text.h \
                      atf-c/ui.c \
                      atf-c/ui.h \
                      atf-c/user.c \
                      atf-c/user.h \
                      atf-c/tc.c \
                      atf-c/tc.h \
                      atf-c/tp.c \
                      atf-c/tp.h \
                      atf-c/tp_main.c
nodist_libatf_c_la_SOURCES = atf-c/defs.h
libatf_c_la_CPPFLAGS = "-DATF_ARCH=\"$(atf_arch)\"" \
                       "-DATF_BUILD_CC=\"$(ATF_BUILD_CC)\"" \
                       "-DATF_BUILD_CFLAGS=\"$(ATF_BUILD_CFLAGS)\"" \
                       "-DATF_BUILD_CPP=\"$(ATF_BUILD_CPP)\"" \
                       "-DATF_BUILD_CPPFLAGS=\"$(ATF_BUILD_CPPFLAGS)\"" \
                       "-DATF_BUILD_CXX=\"$(ATF_BUILD_CXX)\"" \
                       "-DATF_BUILD_CXXFLAGS=\"$(ATF_BUILD_CXXFLAGS)\"" \
                       "-DATF_CONFDIR=\"$(atf_confdir)\"" \
                       "-DATF_INCLUDEDIR=\"$(includedir)\"" \
                       "-DATF_LIBDIR=\"$(libdir)\"" \
                       "-DATF_LIBEXECDIR=\"$(libexecdir)\"" \
                       "-DATF_MACHINE=\"$(atf_machine)\"" \
                       "-DATF_M4=\"$(ATF_M4)\"" \
                       "-DATF_PKGDATADIR=\"$(pkgdatadir)\"" \
                       "-DATF_SHELL=\"$(ATF_SHELL)\"" \
                       "-DATF_WORKDIR=\"$(ATF_WORKDIR)\""


# XXX For some reason, the nodist line above does not work as expected.
# Work this problem around.
dist-hook: kill-defs-h
kill-defs-h:
	rm -f $(distdir)/atf-c/defs.h

include_HEADERS += atf-c.h
atf_c_HEADERS = atf-c/build.h \
                atf-c/check.h \
                atf-c/config.h \
                atf-c/defs.h \
                atf-c/dynstr.h \
                atf-c/env.h \
                atf-c/error.h \
                atf-c/error_fwd.h \
                atf-c/fs.h \
                atf-c/list.h \
                atf-c/macros.h \
                atf-c/map.h \
                atf-c/process.h \
                atf-c/sanity.h \
                atf-c/tc.h \
                atf-c/text.h \
                atf-c/tp.h \
                atf-c/ui.h \
                atf-c/user.h
atf_cdir = $(includedir)/atf-c

dist_man_MANS += atf-c/atf-c-api.3

atf_cpkgconfigdir = $(atf_pkgconfigdir)
atf_cpkgconfig_DATA = atf-c/atf-c.pc
CLEANFILES += atf-c/atf-c.pc
EXTRA_DIST += atf-c/atf-c.pc.in
atf-c/atf-c.pc: $(srcdir)/atf-c/atf-c.pc.in
	test -d atf-c || mkdir -p atf-c
	sed -e 's,__ATF_VERSION__,@PACKAGE_VERSION@,g' \
	    -e 's,__CC__,$(CC),g' \
	    -e 's,__INCLUDEDIR__,$(includedir),g' \
	    -e 's,__LIBDIR__,$(libdir),g' \
	    <$(srcdir)/atf-c/atf-c.pc.in >atf-c/atf-c.pc.tmp
	mv atf-c/atf-c.pc.tmp atf-c/atf-c.pc

tests_atf_c_DATA = atf-c/Atffile \
                   atf-c/d_use_macros_h.c
tests_atf_cdir = $(pkgtestsdir)/atf-c
EXTRA_DIST += $(tests_atf_c_DATA)

noinst_LTLIBRARIES += atf-c/libh.la
atf_c_libh_la_SOURCES = atf-c/h_lib.c \
                        atf-c/h_lib.h

C_TP([atf-c], [atf_c_test], [], [atf-c/libh.la])
C_TP([atf-c], [build_test], [atf-c/h_build.h], [atf-c/libh.la])
C_TP([atf-c], [check_test], [], [atf-c/libh.la])
C_TP([atf-c], [config_test], [], [atf-c/libh.la])
C_TP([atf-c], [dynstr_test], [], [atf-c/libh.la])
C_TP([atf-c], [env_test], [], [atf-c/libh.la])
C_TP([atf-c], [error_test], [], [atf-c/libh.la])
C_TP([atf-c], [fs_test], [], [atf-c/libh.la])
C_TP([atf-c], [h_lib_test], [], [atf-c/libh.la])
C_TP([atf-c], [list_test], [], [atf-c/libh.la])
C_TP([atf-c], [macros_test], [], [atf-c/libh.la])
C_TP([atf-c], [map_test], [], [atf-c/libh.la])
C_TP([atf-c], [process_test], [], [atf-c/libh.la])
C_TP([atf-c], [tc_test], [], [atf-c/libh.la])
C_TP([atf-c], [sanity_test], [], [atf-c/libh.la])
C_TP([atf-c], [text_test], [], [atf-c/libh.la])
C_TP([atf-c], [tp_test], [], [atf-c/libh.la])
C_TP([atf-c], [ui_test], [], [atf-c/libh.la])
C_TP([atf-c], [user_test], [], [atf-c/libh.la])
SH_TP([atf-c], [pkg_config_test])

tests_atf_c_PROGRAMS += atf-c/h_processes
atf_c_h_processes_SOURCES = atf-c/h_processes.c

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
