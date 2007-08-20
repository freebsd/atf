#
# Automated Testing Framework (atf)
#
# Copyright (c) 2007 The NetBSD Foundation, Inc.
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

doc_DATA = AUTHORS COPYING NEWS README ROADMAP
EXTRA_DIST = $(doc_DATA)

dist-hook: check-readme check-style

AM_CPPFLAGS = -DATF_CONFDIR=\"$(atf_confdir)\" \
              -DATF_LIBEXECDIR=\"$(libexecdir)\" \
              -DATF_PKGDATADIR=\"$(pkgdatadir)\" \
              -DATF_SHELL=\"$(ATF_SHELL)\" \
              -DATF_WORKDIR=\"$(ATF_WORKDIR)\"

ATF_COMPILE_DEPS = $(srcdir)/subrs/atf.init.subr
ATF_COMPILE_DEPS += tools/atf-host-compile
ATF_COMPILE_SH = ./tools/atf-host-compile

# -------------------------------------------------------------------------
# `admin' directory.
# -------------------------------------------------------------------------

.PHONY: check-readme
check-readme:
	$(srcdir)/admin/check-readme.sh $(srcdir)/README

.phony: check-style
check-style:
	$(srcdir)/admin/check-style.sh

EXTRA_DIST += admin/check-style-common.awk \
              admin/check-style-cpp.awk \
              admin/check-style-shell.awk \
              admin/check-style.sh

# -------------------------------------------------------------------------
# `atf' directory.
# -------------------------------------------------------------------------

lib_LIBRARIES = libatf.a
libatf_a_SOURCES =  atf/application.cpp \
                    atf/application.hpp \
                    atf/atffile.cpp \
                    atf/atffile.hpp \
                    atf/config.cpp \
                    atf/config.hpp \
                    atf/env.cpp \
                    atf/env.hpp \
                    atf/exceptions.cpp \
                    atf/exceptions.hpp \
                    atf/expand.cpp \
                    atf/expand.hpp \
                    atf/formats.cpp \
                    atf/formats.hpp \
                    atf/fs.cpp \
                    atf/fs.hpp \
                    atf/io.cpp \
                    atf/io.hpp \
                    atf/macros.hpp \
                    atf/serial.cpp \
                    atf/serial.hpp \
                    atf/tests.cpp \
                    atf/tests.hpp \
                    atf/text.cpp \
                    atf/text.hpp \
                    atf/ui.cpp \
                    atf/ui.hpp \
                    atf/user.cpp \
                    atf/user.hpp

include_HEADERS = atf.hpp
atf_HEADERS = atf/application.hpp \
              atf/atffile.hpp \
              atf/config.hpp \
              atf/env.hpp \
              atf/exceptions.hpp \
              atf/expand.hpp \
              atf/formats.hpp \
              atf/fs.hpp \
              atf/io.hpp \
              atf/serial.hpp \
              atf/tests.hpp \
              atf/text.hpp \
              atf/ui.hpp \
              atf/user.hpp
atfdir = $(includedir)/atf

# -------------------------------------------------------------------------
# `doc' directory.
# -------------------------------------------------------------------------

man_MANS = doc/atf.7
CLEANFILES = doc/atf.7
EXTRA_DIST += doc/atf.7.in

dist_man_MANS = doc/atf-formats.5 \
                doc/atf-test-case.4 \
                doc/atf-test-program.1

doc/atf.7: $(srcdir)/doc/atf.7.in
	test -d doc || mkdir -p doc
	sed -e 's,__DOCDIR__,$(docdir),g' \
	    <$(srcdir)/doc/atf.7.in >doc/atf.7.tmp
	mv doc/atf.7.tmp doc/atf.7

# -------------------------------------------------------------------------
# `subrs' directory.
# -------------------------------------------------------------------------

subrs_DATA = subrs/atf.footer.subr \
             subrs/atf.header.subr \
             subrs/atf.init.subr
subrsdir = $(pkgdatadir)
EXTRA_DIST += $(subrs_DATA)

# -------------------------------------------------------------------------
# `tests/bootstrap' directory.
# -------------------------------------------------------------------------

check_PROGRAMS = tests/bootstrap/h_app_empty
tests_bootstrap_h_app_empty_SOURCES = tests/bootstrap/h_app_empty.cpp
tests_bootstrap_h_app_empty_LDADD = -latf
tests_bootstrap_h_app_empty_LDFLAGS = -L.

check_PROGRAMS += tests/bootstrap/h_app_opts_args
tests_bootstrap_h_app_opts_args_SOURCES = tests/bootstrap/h_app_opts_args.cpp
tests_bootstrap_h_app_opts_args_LDADD = -latf
tests_bootstrap_h_app_opts_args_LDFLAGS = -L.

check_PROGRAMS += tests/bootstrap/h_tp_basic_cpp
tests_bootstrap_h_tp_basic_cpp_SOURCES = tests/bootstrap/h_tp_basic_cpp.cpp
tests_bootstrap_h_tp_basic_cpp_LDADD = -latf
tests_bootstrap_h_tp_basic_cpp_LDFLAGS = -L.

check_SCRIPTS = tests/bootstrap/h_tp_basic_sh
CLEANFILES += tests/bootstrap/h_tp_basic_sh
EXTRA_DIST += tests/bootstrap/h_tp_basic_sh.sh
tests/bootstrap/h_tp_basic_sh: $(srcdir)/tests/bootstrap/h_tp_basic_sh.sh \
                               $(ATF_COMPILE_DEPS)
	test -d tests/bootstrap || mkdir -p tests/bootstrap
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/bootstrap/h_tp_basic_sh.sh

check_SCRIPTS += tests/bootstrap/h_tp_atf_check_sh
CLEANFILES += tests/bootstrap/h_tp_atf_check_sh
EXTRA_DIST += tests/bootstrap/h_tp_atf_check_sh.sh
tests/bootstrap/h_tp_atf_check_sh: \
		$(srcdir)/tests/bootstrap/h_tp_atf_check_sh.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/bootstrap || mkdir -p tests/bootstrap
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/bootstrap/h_tp_atf_check_sh.sh

TESTS=		$(srcdir)/tests/bootstrap/testsuite

DISTCLEANFILES = \
		tests/bootstrap/atconfig \
		testsuite.lineno \
		testsuite.log

distclean-local:
	-rm -rf testsuite.dir

EXTRA_DIST +=	tests/bootstrap/testsuite \
		tests/bootstrap/package.m4 \
		tests/bootstrap/testsuite.at \
		$(testsuite_incs)

testsuite_incs=	$(srcdir)/tests/bootstrap/t_application_help.at \
		$(srcdir)/tests/bootstrap/t_application_opts_args.at \
		$(srcdir)/tests/bootstrap/t_atf_config.at \
		$(srcdir)/tests/bootstrap/t_atf_format.at \
		$(srcdir)/tests/bootstrap/t_atf_run.at \
		$(srcdir)/tests/bootstrap/t_subr_atf_check.at \
		$(srcdir)/tests/bootstrap/t_test_program_compare.at \
		$(srcdir)/tests/bootstrap/t_test_program_filter.at \
		$(srcdir)/tests/bootstrap/t_test_program_list.at \
		$(srcdir)/tests/bootstrap/t_test_program_run.at

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
# `tests/atf' directory.
# -------------------------------------------------------------------------

check-local:
	cd tests/atf && $(TESTS_ENVIRONMENT) atf-run | atf-report

testsdir = $(exec_prefix)/tests
pkgtestsdir = $(exec_prefix)/tests/atf

pkgtests_DATA = tests/atf/Atffile
EXTRA_DIST += $(pkgtests_DATA)

atf_sh_interface_DATA = tests/atf/sh_interface/Atffile
atf_sh_interfacedir = $(pkgtestsdir)/sh_interface
EXTRA_DIST += $(atf_sh_interface_DATA)

atf_sh_interface_SCRIPTS = tests/atf/sh_interface/t_atf_check
CLEANFILES += tests/atf/sh_interface/t_atf_check
EXTRA_DIST += tests/atf/sh_interface/t_atf_check.sh
tests/atf/sh_interface/t_atf_check: \
		$(srcdir)/tests/atf/sh_interface/t_atf_check.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/sh_interface || mkdir -p tests/atf/sh_interface
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/sh_interface/t_atf_check.sh

atf_sh_interface_SCRIPTS += tests/atf/sh_interface/t_config
CLEANFILES += tests/atf/sh_interface/t_config
EXTRA_DIST += tests/atf/sh_interface/t_config.sh
tests/atf/sh_interface/t_config: \
		$(srcdir)/tests/atf/sh_interface/t_config.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/sh_interface || mkdir -p tests/atf/sh_interface
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/sh_interface/t_config.sh

atf_sh_interface_SCRIPTS += tests/atf/sh_interface/t_normalize
CLEANFILES += tests/atf/sh_interface/t_normalize
EXTRA_DIST += tests/atf/sh_interface/t_normalize.sh
tests/atf/sh_interface/t_normalize: \
		$(srcdir)/tests/atf/sh_interface/t_normalize.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/sh_interface || mkdir -p tests/atf/sh_interface
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/sh_interface/t_normalize.sh

atf_sh_interface_SCRIPTS += tests/atf/sh_interface/t_tc
CLEANFILES += tests/atf/sh_interface/t_tc
EXTRA_DIST += tests/atf/sh_interface/t_tc.sh
tests/atf/sh_interface/t_tc: \
		$(srcdir)/tests/atf/sh_interface/t_tc.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/sh_interface || mkdir -p tests/atf/sh_interface
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/sh_interface/t_tc.sh

atf_test_programs_DATA = tests/atf/test_programs/Atffile
atf_test_programsdir = $(pkgtestsdir)/test_programs
EXTRA_DIST += $(atf_test_programs_DATA)

atf_test_programs_SCRIPTS = tests/atf/test_programs/h_sh
CLEANFILES += tests/atf/test_programs/h_sh
EXTRA_DIST += tests/atf/test_programs/h_sh.sh
tests/atf/test_programs/h_sh: \
		$(srcdir)/tests/atf/test_programs/h_sh.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/test_programs || mkdir -p tests/atf/test_programs
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/h_sh.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_config
CLEANFILES += tests/atf/test_programs/t_config
EXTRA_DIST += tests/atf/test_programs/t_config.sh
tests/atf/test_programs/t_config: \
		$(srcdir)/tests/atf/test_programs/t_config.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/test_programs || mkdir -p tests/atf/test_programs
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_config.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_env
CLEANFILES += tests/atf/test_programs/t_env
EXTRA_DIST += tests/atf/test_programs/t_env.sh
tests/atf/test_programs/t_env: \
		$(srcdir)/tests/atf/test_programs/t_env.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/test_programs || mkdir -p tests/atf/test_programs
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_env.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_fork
CLEANFILES += tests/atf/test_programs/t_fork
EXTRA_DIST += tests/atf/test_programs/t_fork.sh
tests/atf/test_programs/t_fork: \
		$(srcdir)/tests/atf/test_programs/t_fork.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/test_programs || mkdir -p tests/atf/test_programs
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_fork.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_meta_data
CLEANFILES += tests/atf/test_programs/t_meta_data
EXTRA_DIST += tests/atf/test_programs/t_meta_data.sh
tests/atf/test_programs/t_meta_data: \
		$(srcdir)/tests/atf/test_programs/t_meta_data.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/test_programs || mkdir -p tests/atf/test_programs
	$(ATF_COMPILE_SH) -o $@ \
	    $(srcdir)/tests/atf/test_programs/t_meta_data.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_srcdir
CLEANFILES += tests/atf/test_programs/t_srcdir
EXTRA_DIST += tests/atf/test_programs/t_srcdir.sh
tests/atf/test_programs/t_srcdir: \
		$(srcdir)/tests/atf/test_programs/t_srcdir.sh \
		$(ATF_COMPILE_DEPS)
	test -d tests/atf/test_programs || mkdir -p tests/atf/test_programs
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_srcdir.sh

atf_test_programs_PROGRAMS = tests/atf/test_programs/h_cpp
tests_atf_test_programs_h_cpp_SOURCES = tests/atf/test_programs/h_cpp.cpp
tests_atf_test_programs_h_cpp_LDADD = -latf
tests_atf_test_programs_h_cpp_LDFLAGS = -L.

atf_tools_DATA = tests/atf/tools/Atffile
atf_toolsdir = $(pkgtestsdir)/tools
EXTRA_DIST += $(atf_tools_DATA)

atf_tools_SCRIPTS = tests/atf/tools/t_atf_cleanup
CLEANFILES += tests/atf/tools/t_atf_cleanup
EXTRA_DIST += tests/atf/tools/t_atf_cleanup.sh
tests/atf/tools/t_atf_cleanup: \
			$(srcdir)/tests/atf/tools/t_atf_cleanup.sh \
                        $(ATF_COMPILE_DEPS)
	test -d tests/atf/tools || mkdir -p tests/atf/tools
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_cleanup.sh

atf_tools_SCRIPTS += tests/atf/tools/t_atf_compile
CLEANFILES += tests/atf/tools/t_atf_compile
EXTRA_DIST += tests/atf/tools/t_atf_compile.sh
tests/atf/tools/t_atf_compile: $(srcdir)/tests/atf/tools/t_atf_compile.sh \
                               $(ATF_COMPILE_DEPS)
	test -d tests/atf/tools || mkdir -p tests/atf/tools
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_compile.sh

atf_tools_SCRIPTS += tests/atf/tools/t_atf_config
CLEANFILES += tests/atf/tools/t_atf_config
EXTRA_DIST += tests/atf/tools/t_atf_config.sh
tests/atf/tools/t_atf_config: $(srcdir)/tests/atf/tools/t_atf_config.sh \
                               $(ATF_COMPILE_DEPS)
	test -d tests/atf/tools || mkdir -p tests/atf/tools
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_config.sh

atf_tools_SCRIPTS += tests/atf/tools/t_atf_report
CLEANFILES += tests/atf/tools/t_atf_report
EXTRA_DIST += tests/atf/tools/t_atf_report.sh
tests/atf/tools/t_atf_report: $(srcdir)/tests/atf/tools/t_atf_report.sh \
                           $(ATF_COMPILE_DEPS)
	test -d tests/atf/tools || mkdir -p tests/atf/tools
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_report.sh

atf_tools_SCRIPTS += tests/atf/tools/t_atf_run
CLEANFILES += tests/atf/tools/t_atf_run
EXTRA_DIST += tests/atf/tools/t_atf_run.sh
tests/atf/tools/t_atf_run: $(srcdir)/tests/atf/tools/t_atf_run.sh \
                           $(ATF_COMPILE_DEPS)
	test -d tests/atf/tools || mkdir -p tests/atf/tools
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_run.sh

atf_units_DATA = tests/atf/units/Atffile
atf_unitsdir = $(pkgtestsdir)/units
EXTRA_DIST += $(atf_units_DATA)

atf_units_PROGRAMS = tests/atf/units/t_config
tests_atf_units_t_config_SOURCES = tests/atf/units/t_config.cpp
tests_atf_units_t_config_LDADD = -latf
tests_atf_units_t_config_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_env
tests_atf_units_t_env_SOURCES = tests/atf/units/t_env.cpp
tests_atf_units_t_env_LDADD = -latf
tests_atf_units_t_env_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_expand
tests_atf_units_t_expand_SOURCES = tests/atf/units/t_expand.cpp
tests_atf_units_t_expand_LDADD = -latf
tests_atf_units_t_expand_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_formats
tests_atf_units_t_formats_SOURCES = tests/atf/units/t_formats.cpp
tests_atf_units_t_formats_LDADD = -latf
tests_atf_units_t_formats_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_fs
tests_atf_units_t_fs_SOURCES = tests/atf/units/t_fs.cpp
tests_atf_units_t_fs_LDADD = -latf
tests_atf_units_t_fs_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_io
tests_atf_units_t_io_SOURCES = tests/atf/units/t_io.cpp
tests_atf_units_t_io_LDADD = -latf
tests_atf_units_t_io_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_serial
tests_atf_units_t_serial_SOURCES = tests/atf/units/t_serial.cpp
tests_atf_units_t_serial_LDADD = -latf
tests_atf_units_t_serial_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_tests
tests_atf_units_t_tests_SOURCES = tests/atf/units/t_tests.cpp
tests_atf_units_t_tests_LDADD = -latf
tests_atf_units_t_tests_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_text
tests_atf_units_t_text_SOURCES = tests/atf/units/t_text.cpp
tests_atf_units_t_text_LDADD = -latf
tests_atf_units_t_text_LDFLAGS = -L.

atf_units_PROGRAMS += tests/atf/units/t_user
tests_atf_units_t_user_SOURCES = tests/atf/units/t_user.cpp
tests_atf_units_t_user_LDADD = -latf
tests_atf_units_t_user_LDFLAGS = -L.

# -------------------------------------------------------------------------
# `tests/build' directory.
# -------------------------------------------------------------------------

check_LIBRARIES = tests/build/libuseless.a

tests_build_libuseless_a_SOURCES = \
    tests/build/t_include_application_hpp.cpp \
    tests/build/t_include_atf_hpp.cpp \
    tests/build/t_include_atffile_hpp.cpp \
    tests/build/t_include_config_hpp.cpp \
    tests/build/t_include_env_hpp.cpp \
    tests/build/t_include_exceptions_hpp.cpp \
    tests/build/t_include_expand_hpp.cpp \
    tests/build/t_include_formats_hpp.cpp \
    tests/build/t_include_fs_hpp.cpp \
    tests/build/t_include_io_hpp.cpp \
    tests/build/t_include_macros_hpp.cpp \
    tests/build/t_include_serial_hpp.cpp \
    tests/build/t_include_tests_hpp.cpp \
    tests/build/t_include_text_hpp.cpp \
    tests/build/t_include_ui_hpp.cpp \
    tests/build/t_include_user_hpp.cpp

# -------------------------------------------------------------------------
# `tools' directory.
# -------------------------------------------------------------------------

bin_PROGRAMS = tools/atf-config
tools_atf_config_SOURCES = tools/atf-config.cpp
tools_atf_config_LDADD = -latf
tools_atf_config_LDFLAGS = -L.
dist_man_MANS += tools/atf-config.1

tools/atf-host-compile: $(srcdir)/tools/atf-host-compile.sh
	sed -e 's,__ATF_PKGDATADIR__,$(srcdir)/subrs,g' \
	    -e 's,__ATF_SHELL__,$(ATF_SHELL),g' \
	    <$(srcdir)/tools/atf-host-compile.sh \
	    >tools/atf-host-compile.tmp
	chmod +x tools/atf-host-compile.tmp
	mv tools/atf-host-compile.tmp tools/atf-host-compile
CLEANFILES += tools/atf-host-compile
CLEANFILES += tools/atf-host-compile.tmp
EXTRA_DIST += tools/atf-host-compile.sh

libexec_PROGRAMS = tools/atf-cleanup
tools_atf_cleanup_SOURCES = tools/atf-cleanup.cpp
tools_atf_cleanup_LDADD = -latf
tools_atf_cleanup_LDFLAGS = -L.
dist_man_MANS += tools/atf-cleanup.1

bin_PROGRAMS += tools/atf-compile
tools_atf_compile_SOURCES = tools/atf-compile.cpp
tools_atf_compile_LDADD = -latf
tools_atf_compile_LDFLAGS = -L.
dist_man_MANS += tools/atf-compile.1

libexec_PROGRAMS += tools/atf-format
tools_atf_format_SOURCES = tools/atf-format.cpp
tools_atf_format_LDADD = -latf
tools_atf_format_LDFLAGS = -L.
dist_man_MANS += tools/atf-format.1

bin_PROGRAMS += tools/atf-report
tools_atf_report_SOURCES = tools/atf-report.cpp
tools_atf_report_LDADD = -latf
tools_atf_report_LDFLAGS = -L.
dist_man_MANS += tools/atf-report.1

bin_PROGRAMS += tools/atf-run
tools_atf_run_SOURCES = tools/atf-run.cpp
tools_atf_run_LDADD = -latf
tools_atf_run_LDFLAGS = -L.
dist_man_MANS += tools/atf-run.1

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
