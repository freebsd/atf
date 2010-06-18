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

tests_atf_c___PROGRAMS = atf-c++/atf_c++_test
atf_c___atf_c___test_SOURCES = atf-c++/atf_c++_test.cpp
atf_c___atf_c___test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/application_test
atf_c___application_test_SOURCES = atf-c++/application_test.cpp
atf_c___application_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/build_test
atf_c___build_test_SOURCES = atf-c++/build_test.cpp atf-c/h_build.h
atf_c___build_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/check_test
atf_c___check_test_SOURCES = atf-c++/check_test.cpp
atf_c___check_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/config_test
atf_c___config_test_SOURCES = atf-c++/config_test.cpp
atf_c___config_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/env_test
atf_c___env_test_SOURCES = atf-c++/env_test.cpp
atf_c___env_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/exceptions_test
atf_c___exceptions_test_SOURCES = atf-c++/exceptions_test.cpp
atf_c___exceptions_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/expand_test
atf_c___expand_test_SOURCES = atf-c++/expand_test.cpp
atf_c___expand_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/fs_test
atf_c___fs_test_SOURCES = atf-c++/fs_test.cpp
atf_c___fs_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/io_test
atf_c___io_test_SOURCES = atf-c++/io_test.cpp
atf_c___io_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/macros_test
atf_c___macros_test_SOURCES = atf-c++/macros_test.cpp
atf_c___macros_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/parser_test
atf_c___parser_test_SOURCES = atf-c++/parser_test.cpp
atf_c___parser_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___SCRIPTS = atf-c++/pkg_config_test
CLEANFILES += atf-c++/pkg_config_test
EXTRA_DIST += atf-c++/pkg_config_test.sh
atf-c++/pkg_config_test: $(srcdir)/atf-c++/pkg_config_test.sh
	test -d atf-c++ || mkdir -p atf-c++
	@src="$(srcdir)/atf-c++/pkg_config_test.sh"; \
	dst="atf-c++/pkg_config_test"; $(BUILD_SH_TP)

tests_atf_c___PROGRAMS += atf-c++/process_test
atf_c___process_test_SOURCES = atf-c++/process_test.cpp
atf_c___process_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/sanity_test
atf_c___sanity_test_SOURCES = atf-c++/sanity_test.cpp
atf_c___sanity_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/signals_test
atf_c___signals_test_SOURCES = atf-c++/signals_test.cpp
atf_c___signals_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/tests_test
atf_c___tests_test_SOURCES = atf-c++/tests_test.cpp
atf_c___tests_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/text_test
atf_c___text_test_SOURCES = atf-c++/text_test.cpp
atf_c___text_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/ui_test
atf_c___ui_test_SOURCES = atf-c++/ui_test.cpp
atf_c___ui_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/user_test
atf_c___user_test_SOURCES = atf-c++/user_test.cpp
atf_c___user_test_LDADD = atf-c++/libh.la libatf-c++.la

tests_atf_c___PROGRAMS += atf-c++/utils_test
atf_c___utils_test_SOURCES = atf-c++/utils_test.cpp
atf_c___utils_test_LDADD = atf-c++/libh.la libatf-c++.la

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
