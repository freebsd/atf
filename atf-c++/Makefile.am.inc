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

lib_LTLIBRARIES += libatf-c++.la
libatf_c___la_LIBADD = libatf-c.la
libatf_c___la_SOURCES = atf-c++/application.cpp \
                        atf-c++/application.hpp \
                        atf-c++/build.cpp \
                        atf-c++/build.hpp \
                        atf-c++/check.cpp \
                        atf-c++/check.hpp \
                        atf-c++/config.cpp \
                        atf-c++/config.hpp \
                        atf-c++/env.cpp \
                        atf-c++/env.hpp \
                        atf-c++/exceptions.cpp \
                        atf-c++/exceptions.hpp \
                        atf-c++/expand.cpp \
                        atf-c++/expand.hpp \
                        atf-c++/fs.cpp \
                        atf-c++/fs.hpp \
                        atf-c++/io.cpp \
                        atf-c++/io.hpp \
                        atf-c++/macros.hpp \
                        atf-c++/parser.cpp \
                        atf-c++/parser.hpp \
                        atf-c++/process.cpp \
                        atf-c++/process.hpp \
                        atf-c++/sanity.hpp \
                        atf-c++/signals.cpp \
                        atf-c++/signals.hpp \
                        atf-c++/tests.cpp \
                        atf-c++/tests.hpp \
                        atf-c++/text.cpp \
                        atf-c++/text.hpp \
                        atf-c++/ui.cpp \
                        atf-c++/ui.hpp \
                        atf-c++/user.cpp \
                        atf-c++/user.hpp \
                        atf-c++/utils.hpp

include_HEADERS += atf-c++.hpp
atf_c___HEADERS = atf-c++/application.hpp \
                  atf-c++/build.hpp \
                  atf-c++/check.hpp \
                  atf-c++/config.hpp \
                  atf-c++/env.hpp \
                  atf-c++/exceptions.hpp \
                  atf-c++/expand.hpp \
                  atf-c++/fs.hpp \
                  atf-c++/io.hpp \
                  atf-c++/macros.hpp \
                  atf-c++/parser.hpp \
                  atf-c++/process.hpp \
                  atf-c++/sanity.hpp \
                  atf-c++/signals.hpp \
                  atf-c++/tests.hpp \
                  atf-c++/text.hpp \
                  atf-c++/ui.hpp \
                  atf-c++/user.hpp \
                  atf-c++/utils.hpp
atf_c__dir = $(includedir)/atf-c++

dist_man_MANS += atf-c++/atf-c++-api.3

atf_c__dirpkgconfigdir = $(atf_pkgconfigdir)
atf_c__dirpkgconfig_DATA = atf-c++/atf-c++.pc
CLEANFILES += atf-c++/atf-c++.pc
EXTRA_DIST += atf-c++/atf-c++.pc.in
atf-c++/atf-c++.pc: $(srcdir)/atf-c++/atf-c++.pc.in
	test -d atf-c++ || mkdir -p atf-c++
	sed -e 's,__ATF_VERSION__,@PACKAGE_VERSION@,g' \
	    -e 's,__CXX__,$(CXX),g' \
	    -e 's,__INCLUDEDIR__,$(includedir),g' \
	    -e 's,__LIBDIR__,$(libdir),g' \
	    <$(srcdir)/atf-c++/atf-c++.pc.in >atf-c++/atf-c++.pc.tmp
	mv atf-c++/atf-c++.pc.tmp atf-c++/atf-c++.pc

tests_atf_c___DATA = atf-c++/Atffile \
                   atf-c++/d_use_macros_hpp.cpp
tests_atf_c__dir = $(pkgtestsdir)/atf-c++
EXTRA_DIST += $(tests_atf_c___DATA)

noinst_LTLIBRARIES += atf-c++/libh.la
atf_c___libh_la_SOURCES = atf-c++/h_lib.cpp \
                          atf-c++/h_lib.hpp

CXX_TP([atf-c++], [atf_c++_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [application_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [build_test], [atf-c/h_build.h], [atf-c++/libh.la])
CXX_TP([atf-c++], [check_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [config_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [env_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [exceptions_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [expand_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [fs_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [io_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [macros_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [parser_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [process_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [sanity_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [signals_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [tests_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [text_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [ui_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [user_test], [], [atf-c++/libh.la])
CXX_TP([atf-c++], [utils_test], [], [atf-c++/libh.la])
SH_TP([atf-c++], [pkg_config_test])

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
