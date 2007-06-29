#
# Automated Testing Framework (atf)
#
# Copyright (c) 2007 The NetBSD Foundation, Inc.
# All rights reserved.
#
# This code is derived from software contributed to The NetBSD Foundation
# by Julio M. Merino Vidal, developed as part of Google's Summer of Code
# 2007 program.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this
#    software must display the following acknowledgement:
#        This product includes software developed by the NetBSD
#        Foundation, Inc. and its contributors.
# 4. Neither the name of The NetBSD Foundation nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
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

# -------------------------------------------------------------------------
# Top directory.
# -------------------------------------------------------------------------

if INSTALL_DOCS
doc_DATA = AUTHORS COPYING NEWS README TODO
endif # INSTALL_DOCS

dist-hook:
	@echo "-------------------------------------------------"
	@echo " REMEMBER THAT README HAS TO BE UPDATED WITH THE"
	@echo "     RIGHT VERSIONS OF THE GNU BUILD SYSTEM!"
	@echo "-------------------------------------------------"

AM_CPPFLAGS = -DATF_LIBEXECDIR=\"$(libexecdir)\" \
              -DATF_PKGDATADIR=\"$(pkgdatadir)\"

# -------------------------------------------------------------------------
# `libatf' directory.
# -------------------------------------------------------------------------

lib_LTLIBRARIES = libatf/libatf.la
libatf_libatf_la_SOURCES = libatf/test_case.cpp \
                           libatf/test_case_result.cpp \
                           libatf/variables.cpp

include_HEADERS = libatf.hpp
nobase_include_HEADERS = libatf/exceptions.hpp \
                         libatf/test_case.hpp \
                         libatf/test_case_result.hpp \
                         libatf/variables.hpp

# -------------------------------------------------------------------------
# `libatfmain' directory.
# -------------------------------------------------------------------------

common_sources = libatfmain/application.cpp \
                 libatfmain/atffile.cpp \
                 libatfmain/exceptions.cpp \
                 libatfmain/filesystem.cpp \
                 libatfmain/report.cpp \
                 libatfmain/ui.cpp

noinst_LIBRARIES = libatfmain/libcommon.a
libatfmain_libcommon_a_SOURCES = $(common_sources)
# XXX Workaround for a bug in GNU Automake which prevents the common files
# to be built with and without GNU Libtool.  Setting different CXXFLAGS
# forces the generated Makefile to use different objects for each case.
libatfmain_libcommon_a_CXXFLAGS = $(AM_CXXFLAGS)

lib_LTLIBRARIES += libatfmain/libatfmain.la
libatfmain_libatfmain_la_SOURCES = libatfmain/main.cpp \
                                   $(common_sources)
libatfmain_libatfmain_la_LDFLAGS = -Llibatf
libatfmain_libatfmain_la_LIBADD = -latf

include_HEADERS += libatfmain.hpp

# -------------------------------------------------------------------------
# `tests/bootstrap' directory.
# -------------------------------------------------------------------------

# XXX These should only be built when running 'make check', or something
# like that.

noinst_PROGRAMS = tests/bootstrap/h_app_empty
tests_bootstrap_h_app_empty_SOURCES = tests/bootstrap/h_app_empty.cpp
tests_bootstrap_h_app_empty_LDADD = -latf libatfmain/libcommon.a
tests_bootstrap_h_app_empty_LDFLAGS = -Llibatf

noinst_PROGRAMS += tests/bootstrap/h_app_opts_args
tests_bootstrap_h_app_opts_args_SOURCES = tests/bootstrap/h_app_opts_args.cpp
tests_bootstrap_h_app_opts_args_LDADD = -latf libatfmain/libcommon.a
tests_bootstrap_h_app_opts_args_LDFLAGS = -Llibatf

noinst_PROGRAMS += tests/bootstrap/h_tp_basic
tests_bootstrap_h_tp_basic_SOURCES = tests/bootstrap/h_tp_basic.cpp
tests_bootstrap_h_tp_basic_LDADD = -latf -latfmain
tests_bootstrap_h_tp_basic_LDFLAGS = -Llibatf -Llibatfmain

TESTS=		$(srcdir)/tests/bootstrap/testsuite
TESTS_ENVIRONMENT = PATH=$$(atf-config -t atf_libexecdir):$${PATH}

DISTCLEANFILES = \
		tests/bootstrap/atconfig \
		testsuite.lineno \
		testsuite.log

distclean-local:
	-rm -rf testsuite.dir

EXTRA_DIST =	tests/bootstrap/testsuite \
		tests/bootstrap/package.m4 \
		tests/bootstrap/testsuite.at \
		$(testsuite_incs)

testsuite_incs=	$(srcdir)/tests/bootstrap/t_atf_config.at \
		$(srcdir)/tests/bootstrap/t_atf_format.at \
		$(srcdir)/tests/bootstrap/t_atf_identify.at \
		$(srcdir)/tests/bootstrap/t_libatfmain_filter.at \
		$(srcdir)/tests/bootstrap/t_libatfmain_help.at \
		$(srcdir)/tests/bootstrap/t_libatfmain_list.at \
		$(srcdir)/tests/bootstrap/t_libatfmain_options.at \
		$(srcdir)/tests/bootstrap/t_libatfmain_run.at \
		$(srcdir)/tests/bootstrap/t_libcommon_opts_args.at

$(srcdir)/tests/bootstrap/package.m4: $(top_srcdir)/configure.ac
	{ \
	echo '# Signature of the current package.'; \
	echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/tests/bootstrap/package.m4

$(srcdir)/tests/bootstrap/testsuite: $(srcdir)/tests/bootstrap/testsuite.at \
                                     $(testsuite_incs) \
                                     $(srcdir)/tests/bootstrap/package.m4
	autom4te --language=Autotest -I $(srcdir) \
	    -I $(srcdir)/tests/bootstrap \
	    $(srcdir)/tests/bootstrap/testsuite.at -o $@.tmp
	mv $@.tmp $@

# -------------------------------------------------------------------------
# `tools' directory.
# -------------------------------------------------------------------------

bin_PROGRAMS = tools/atf-config
tools_atf_config_SOURCES = tools/atf-config.cpp
tools_atf_config_LDADD = libatfmain/libcommon.a

libexec_PROGRAMS = tools/atf-format
tools_atf_format_SOURCES = tools/atf-format.cpp
tools_atf_format_LDADD = libatfmain/libcommon.a

libexec_PROGRAMS += tools/atf-identify
tools_atf_identify_SOURCES = tools/atf-identify.cpp
tools_atf_identify_LDADD = libatfmain/libcommon.a

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
