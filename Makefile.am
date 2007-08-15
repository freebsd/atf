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

if INSTALL_DOCS
doc_DATA = AUTHORS COPYING NEWS README TODO
endif # INSTALL_DOCS

dist-hook: check-readme check-style

AM_CPPFLAGS = -DATF_CONFDIR=\"$(atf_confdir)\" \
              -DATF_LIBEXECDIR=\"$(libexecdir)\" \
              -DATF_PKGDATADIR=\"$(pkgdatadir)\" \
              -DATF_SHELL=\"$(ATF_SHELL)\" \
              -DATF_WORKDIR=\"$(ATF_WORKDIR)\" \
              -I$(srcdir)/libs

ATF_COMPILE_DEPS = $(srcdir)/libs/shell/atf.init.subr
ATF_COMPILE_DEPS += tools/atf-host-compile
ATF_COMPILE_SH = ./tools/atf-host-compile

# -------------------------------------------------------------------------
# `admin' directory.
# -------------------------------------------------------------------------

.PHONY: check-readme
check-readme:
	$(srcdir)/admin/check-readme.sh $(srcdir)/README

.PHONY: check-style
check-style:
	$(srcdir)/admin/check-style.sh

EXTRA_DIST = admin/check-style-common.awk \
             admin/check-style-cpp.awk \
             admin/check-style-shell.awk \
             admin/check-style.sh

# -------------------------------------------------------------------------
# `libs/atf' directory.
# -------------------------------------------------------------------------

libs/atf/tcenv.cpp: $(srcdir)/libs/tc-env.sh \
                     $(srcdir)/libs/tc-env.csv
	$(ATF_SHELL) $(srcdir)/libs/tc-env.sh cpp \
	    <$(srcdir)/libs/tc-env.csv \
	    >libs/atf/tcenv.cpp.tmp
	mv libs/atf/tcenv.cpp.tmp libs/atf/tcenv.cpp
CLEANFILES = libs/atf/tcenv.cpp.tmp
CLEANFILES += libs/atf/tcenv.cpp
EXTRA_DIST += libs/tc-env.csv
EXTRA_DIST += libs/tc-env.sh

libs/atf/tchead.cpp: $(srcdir)/libs/tc-meta-data.sh \
                     $(srcdir)/libs/tc-meta-data.csv
	$(ATF_SHELL) $(srcdir)/libs/tc-meta-data.sh cpp \
	    <$(srcdir)/libs/tc-meta-data.csv \
	    >libs/atf/tchead.cpp.tmp
	mv libs/atf/tchead.cpp.tmp libs/atf/tchead.cpp
CLEANFILES += libs/atf/tchead.cpp.tmp
CLEANFILES += libs/atf/tchead.cpp
EXTRA_DIST += libs/tc-meta-data.csv
EXTRA_DIST += libs/tc-meta-data.sh

$(srcdir)/libs/atf/tests.cpp: libs/atf/tcenv.cpp libs/atf/tchead.cpp

lib_LTLIBRARIES = libs/libatf.la
libs_libatf_la_SOURCES = libs/atf/application.cpp \
                         libs/atf/application.hpp \
                         libs/atf/atffile.cpp \
                         libs/atf/atffile.hpp \
                         libs/atf/config.cpp \
                         libs/atf/config.hpp \
                         libs/atf/env.cpp \
                         libs/atf/env.hpp \
                         libs/atf/exceptions.cpp \
                         libs/atf/exceptions.hpp \
                         libs/atf/expand.cpp \
                         libs/atf/expand.hpp \
                         libs/atf/formats.cpp \
                         libs/atf/formats.hpp \
                         libs/atf/fs.cpp \
                         libs/atf/fs.hpp \
                         libs/atf/io.cpp \
                         libs/atf/io.hpp \
                         libs/atf/macros.hpp \
                         libs/atf/serial.cpp \
                         libs/atf/serial.hpp \
                         libs/atf/tests.cpp \
                         libs/atf/tests.hpp \
                         libs/atf/text.cpp \
                         libs/atf/text.hpp \
                         libs/atf/ui.cpp \
                         libs/atf/ui.hpp \
                         libs/atf/user.cpp \
                         libs/atf/user.hpp

include_HEADERS = libs/atf.hpp
atf_HEADERS = libs/atf/application.hpp \
              libs/atf/atffile.hpp \
              libs/atf/config.hpp \
              libs/atf/env.hpp \
              libs/atf/exceptions.hpp \
              libs/atf/expand.hpp \
              libs/atf/formats.hpp \
              libs/atf/fs.hpp \
              libs/atf/io.hpp \
              libs/atf/serial.hpp \
              libs/atf/tests.hpp \
              libs/atf/text.hpp \
              libs/atf/ui.hpp \
              libs/atf/user.hpp
atfdir = $(includedir)/atf

# -------------------------------------------------------------------------
# `libs/shell' directory.
# -------------------------------------------------------------------------

libs/shell/atf.tcenv.subr: $(srcdir)/libs/tc-env.sh \
                            $(srcdir)/libs/tc-env.csv
	$(ATF_SHELL) $(srcdir)/libs/tc-env.sh sh \
	    <$(srcdir)/libs/tc-env.csv \
	    >libs/shell/atf.tcenv.subr.tmp
	mv libs/shell/atf.tcenv.subr.tmp libs/shell/atf.tcenv.subr
CLEANFILES += libs/shell/atf.tcenv.subr.tmp
CLEANFILES += libs/shell/atf.tcenv.subr

libs/shell/atf.tchead.subr: $(srcdir)/libs/tc-meta-data.sh \
                            $(srcdir)/libs/tc-meta-data.csv
	$(ATF_SHELL) $(srcdir)/libs/tc-meta-data.sh sh \
	    <$(srcdir)/libs/tc-meta-data.csv \
	    >libs/shell/atf.tchead.subr.tmp
	mv libs/shell/atf.tchead.subr.tmp libs/shell/atf.tchead.subr
CLEANFILES += libs/shell/atf.tchead.subr.tmp
CLEANFILES += libs/shell/atf.tchead.subr

libs_shell_DATA = libs/shell/atf.footer.subr \
                  libs/shell/atf.header.subr \
                  libs/shell/atf.init.subr \
                  libs/shell/atf.tcenv.subr \
                  libs/shell/atf.tchead.subr
libs_shelldir = $(pkgdatadir)
EXTRA_DIST += $(libs_shell_DATA)

# -------------------------------------------------------------------------
# `tests/bootstrap' directory.
# -------------------------------------------------------------------------

check_PROGRAMS = tests/bootstrap/h_app_empty
tests_bootstrap_h_app_empty_SOURCES = tests/bootstrap/h_app_empty.cpp
tests_bootstrap_h_app_empty_LDADD = -latf
tests_bootstrap_h_app_empty_LDFLAGS = -Llibs

check_PROGRAMS += tests/bootstrap/h_app_opts_args
tests_bootstrap_h_app_opts_args_SOURCES = tests/bootstrap/h_app_opts_args.cpp
tests_bootstrap_h_app_opts_args_LDADD = -latf
tests_bootstrap_h_app_opts_args_LDFLAGS = -Llibs

check_PROGRAMS += tests/bootstrap/h_tp_basic_cpp
tests_bootstrap_h_tp_basic_cpp_SOURCES = tests/bootstrap/h_tp_basic_cpp.cpp
tests_bootstrap_h_tp_basic_cpp_LDADD = -latf
tests_bootstrap_h_tp_basic_cpp_LDFLAGS = -Llibs

check_SCRIPTS = tests/bootstrap/h_tp_basic_sh
CLEANFILES += tests/bootstrap/h_tp_basic_sh
EXTRA_DIST += tests/bootstrap/h_tp_basic_sh.sh
tests/bootstrap/h_tp_basic_sh: $(srcdir)/tests/bootstrap/h_tp_basic_sh.sh \
                               $(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/bootstrap/h_tp_basic_sh.sh

check_SCRIPTS += tests/bootstrap/h_tp_atf_check_sh
CLEANFILES += tests/bootstrap/h_tp_atf_check_sh
EXTRA_DIST += tests/bootstrap/h_tp_atf_check_sh.sh
tests/bootstrap/h_tp_atf_check_sh: \
		$(srcdir)/tests/bootstrap/h_tp_atf_check_sh.sh \
		$(ATF_COMPILE_DEPS)
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
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/sh_interface/t_atf_check.sh

atf_sh_interface_SCRIPTS += tests/atf/sh_interface/t_config
CLEANFILES += tests/atf/sh_interface/t_config
EXTRA_DIST += tests/atf/sh_interface/t_config.sh
tests/atf/sh_interface/t_config: \
		$(srcdir)/tests/atf/sh_interface/t_config.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/sh_interface/t_config.sh

atf_sh_interface_SCRIPTS += tests/atf/sh_interface/t_tc
CLEANFILES += tests/atf/sh_interface/t_tc
EXTRA_DIST += tests/atf/sh_interface/t_tc.sh
tests/atf/sh_interface/t_tc: \
		$(srcdir)/tests/atf/sh_interface/t_tc.sh \
		$(ATF_COMPILE_DEPS)
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
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/h_sh.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_config
CLEANFILES += tests/atf/test_programs/t_config
EXTRA_DIST += tests/atf/test_programs/t_config.sh
tests/atf/test_programs/t_config: \
		$(srcdir)/tests/atf/test_programs/t_config.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_config.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_env
CLEANFILES += tests/atf/test_programs/t_env
EXTRA_DIST += tests/atf/test_programs/t_env.sh
tests/atf/test_programs/t_env: \
		$(srcdir)/tests/atf/test_programs/t_env.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_env.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_fork
CLEANFILES += tests/atf/test_programs/t_fork
EXTRA_DIST += tests/atf/test_programs/t_fork.sh
tests/atf/test_programs/t_fork: \
		$(srcdir)/tests/atf/test_programs/t_fork.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_fork.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_isolated
CLEANFILES += tests/atf/test_programs/t_isolated
EXTRA_DIST += tests/atf/test_programs/t_isolated.sh
tests/atf/test_programs/t_isolated: \
		$(srcdir)/tests/atf/test_programs/t_isolated.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_isolated.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_require_progs
CLEANFILES += tests/atf/test_programs/t_require_progs
EXTRA_DIST += tests/atf/test_programs/t_require_progs.sh
tests/atf/test_programs/t_require_progs: \
		$(srcdir)/tests/atf/test_programs/t_require_progs.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ \
	    $(srcdir)/tests/atf/test_programs/t_require_progs.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_require_user
CLEANFILES += tests/atf/test_programs/t_require_user
EXTRA_DIST += tests/atf/test_programs/t_require_user.sh
tests/atf/test_programs/t_require_user: \
		$(srcdir)/tests/atf/test_programs/t_require_user.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ \
	    $(srcdir)/tests/atf/test_programs/t_require_user.sh

atf_test_programs_SCRIPTS += tests/atf/test_programs/t_srcdir
CLEANFILES += tests/atf/test_programs/t_srcdir
EXTRA_DIST += tests/atf/test_programs/t_srcdir.sh
tests/atf/test_programs/t_srcdir: \
		$(srcdir)/tests/atf/test_programs/t_srcdir.sh \
		$(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/test_programs/t_srcdir.sh

atf_test_programs_PROGRAMS = tests/atf/test_programs/h_cpp
tests_atf_test_programs_h_cpp_SOURCES = tests/atf/test_programs/h_cpp.cpp
tests_atf_test_programs_h_cpp_LDADD = -latf
tests_atf_test_programs_h_cpp_LDFLAGS = -Llibs

atf_tools_DATA = tests/atf/tools/Atffile
atf_toolsdir = $(pkgtestsdir)/tools
EXTRA_DIST += $(atf_tools_DATA)

atf_tools_SCRIPTS = tests/atf/tools/t_atf_cleanup
CLEANFILES += tests/atf/tools/t_atf_cleanup
EXTRA_DIST += tests/atf/tools/t_atf_cleanup.sh
tests/atf/tools/t_atf_cleanup: \
			$(srcdir)/tests/atf/tools/t_atf_cleanup.sh \
                        $(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_cleanup.sh

atf_tools_SCRIPTS += tests/atf/tools/t_atf_compile
CLEANFILES += tests/atf/tools/t_atf_compile
EXTRA_DIST += tests/atf/tools/t_atf_compile.sh
tests/atf/tools/t_atf_compile: $(srcdir)/tests/atf/tools/t_atf_compile.sh \
                               $(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_compile.sh

atf_tools_SCRIPTS += tests/atf/tools/t_atf_config
CLEANFILES += tests/atf/tools/t_atf_config
EXTRA_DIST += tests/atf/tools/t_atf_config.sh
tests/atf/tools/t_atf_config: $(srcdir)/tests/atf/tools/t_atf_config.sh \
                               $(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_config.sh

atf_tools_SCRIPTS += tests/atf/tools/t_atf_run
CLEANFILES += tests/atf/tools/t_atf_run
EXTRA_DIST += tests/atf/tools/t_atf_run.sh
tests/atf/tools/t_atf_run: $(srcdir)/tests/atf/tools/t_atf_run.sh \
                           $(ATF_COMPILE_DEPS)
	$(ATF_COMPILE_SH) -o $@ $(srcdir)/tests/atf/tools/t_atf_run.sh

atf_units_DATA = tests/atf/units/Atffile
atf_unitsdir = $(pkgtestsdir)/units
EXTRA_DIST += $(atf_units_DATA)

atf_units_PROGRAMS = tests/atf/units/t_config
tests_atf_units_t_config_SOURCES = tests/atf/units/t_config.cpp
tests_atf_units_t_config_LDADD = -latf
tests_atf_units_t_config_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_env
tests_atf_units_t_env_SOURCES = tests/atf/units/t_env.cpp
tests_atf_units_t_env_LDADD = -latf
tests_atf_units_t_env_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_expand
tests_atf_units_t_expand_SOURCES = tests/atf/units/t_expand.cpp
tests_atf_units_t_expand_LDADD = -latf
tests_atf_units_t_expand_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_formats
tests_atf_units_t_formats_SOURCES = tests/atf/units/t_formats.cpp
tests_atf_units_t_formats_LDADD = -latf
tests_atf_units_t_formats_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_fs
tests_atf_units_t_fs_SOURCES = tests/atf/units/t_fs.cpp
tests_atf_units_t_fs_LDADD = -latf
tests_atf_units_t_fs_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_io
tests_atf_units_t_io_SOURCES = tests/atf/units/t_io.cpp
tests_atf_units_t_io_LDADD = -latf
tests_atf_units_t_io_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_serial
tests_atf_units_t_serial_SOURCES = tests/atf/units/t_serial.cpp
tests_atf_units_t_serial_LDADD = -latf
tests_atf_units_t_serial_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_tests
tests_atf_units_t_tests_SOURCES = tests/atf/units/t_tests.cpp
tests_atf_units_t_tests_LDADD = -latf
tests_atf_units_t_tests_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_text
tests_atf_units_t_text_SOURCES = tests/atf/units/t_text.cpp
tests_atf_units_t_text_LDADD = -latf
tests_atf_units_t_text_LDFLAGS = -Llibs

atf_units_PROGRAMS += tests/atf/units/t_user
tests_atf_units_t_user_SOURCES = tests/atf/units/t_user.cpp
tests_atf_units_t_user_LDADD = -latf
tests_atf_units_t_user_LDFLAGS = -Llibs

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
tools_atf_config_LDFLAGS = -Llibs

tools/atf-host-compile: $(srcdir)/tools/atf-host-compile.sh
	sed -e 's,__ATF_PKGDATADIR__,$(srcdir)/libs/shell,g' \
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
tools_atf_cleanup_LDFLAGS = -Llibs

bin_PROGRAMS += tools/atf-compile
tools_atf_compile_SOURCES = tools/atf-compile.cpp
tools_atf_compile_LDADD = -latf
tools_atf_compile_LDFLAGS = -Llibs

libexec_PROGRAMS += tools/atf-format
tools_atf_format_SOURCES = tools/atf-format.cpp
tools_atf_format_LDADD = -latf
tools_atf_format_LDFLAGS = -Llibs

bin_PROGRAMS += tools/atf-report
tools_atf_report_SOURCES = tools/atf-report.cpp
tools_atf_report_LDADD = -latf
tools_atf_report_LDFLAGS = -Llibs

bin_PROGRAMS += tools/atf-run
tools_atf_run_SOURCES = tools/atf-run.cpp
tools_atf_run_LDADD = -latf
tools_atf_run_LDFLAGS = -Llibs

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
