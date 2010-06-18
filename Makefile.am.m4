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

m4_changequote([, ])

ACLOCAL_AMFLAGS = -I m4

BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
bin_PROGRAMS =
dist_man_MANS =
include_HEADERS =
lib_LTLIBRARIES =
libexec_PROGRAMS =
man_MANS =
noinst_LTLIBRARIES =

#
# Top-level distfile document generation and installation.
#

doc_DATA = AUTHORS COPYING NEWS README
noinst_DATA = INSTALL
CLEANFILES += AUTHORS COPYING INSTALL NEWS README
EXTRA_DIST += $(doc_DATA) INSTALL

$(srcdir)/AUTHORS: $(srcdir)/doc/text/authors.txt
	cp $(srcdir)/doc/text/authors.txt $(srcdir)/AUTHORS

$(srcdir)/COPYING: $(srcdir)/doc/text/copying.txt
	cp $(srcdir)/doc/text/copying.txt $(srcdir)/COPYING

$(srcdir)/INSTALL: $(srcdir)/doc/text/install.txt
	cp $(srcdir)/doc/text/install.txt $(srcdir)/INSTALL

$(srcdir)/NEWS: $(srcdir)/doc/text/news.txt
	cp $(srcdir)/doc/text/news.txt $(srcdir)/NEWS

$(srcdir)/README: $(srcdir)/doc/text/readme.txt
	cp $(srcdir)/doc/text/readme.txt $(srcdir)/README

#
# Makefile.am construction.
#

EXTRA_DIST += Makefile.am.m4
$(srcdir)/Makefile.am: $(srcdir)/admin/generate-makefile.sh \
                       $(srcdir)/Makefile.am.m4
	$(srcdir)/admin/generate-makefile.sh \
	    $(srcdir)/Makefile.am.m4 $(srcdir)/Makefile.am

m4_define([INCLUDE], [
$(srcdir)/Makefile.am: $(srcdir)/$1
m4_include($1)
EXTRA_DIST += $1
])
INCLUDE(admin/Makefile.am.inc)
INCLUDE(atf-c++/Makefile.am.inc)
INCLUDE(atf-c/Makefile.am.inc)
INCLUDE(atf-check/Makefile.am.inc)
INCLUDE(atf-config/Makefile.am.inc)
INCLUDE(atf-format/Makefile.am.inc)
INCLUDE(atf-report/Makefile.am.inc)
INCLUDE(atf-run/Makefile.am.inc)
INCLUDE(atf-sh/Makefile.am.inc)
INCLUDE(atf-version/Makefile.am.inc)
INCLUDE(bootstrap/Makefile.am.inc)
INCLUDE(doc/Makefile.am.inc)
INCLUDE(test-programs/Makefile.am.inc)

#
# Supporting logic to run our custom testsuite.
#

TESTS_ENVIRONMENT = PATH=$(prefix)/bin:$${PATH} \
                    PKG_CONFIG_PATH=$(prefix)/lib/pkgconfig

testsdir = $(exec_prefix)/tests
pkgtestsdir = $(testsdir)/atf

installcheck-local: installcheck-atf
.PHONY: installcheck-atf
installcheck-atf:
	logfile=$$(pwd)/installcheck.log; \
	fifofile=$$(pwd)/installcheck.fifo; \
	cd $(pkgtestsdir); \
	rm -f $${fifofile}; \
	mkfifo $${fifofile}; \
	cat $${fifofile} | tee $${logfile} | $(TESTS_ENVIRONMENT) atf-report & \
	$(TESTS_ENVIRONMENT) atf-run >>$${fifofile}; \
	res=$${?}; \
	wait; \
	rm $${fifofile}; \
	echo; \
	echo "The verbatim output of atf-run has been saved to" \
	     "installcheck.log; exit was $${res}"; \
	test $${res} -eq 0
CLEANFILES += installcheck.fifo installcheck.log

pkgtests_DATA = Atffile
EXTRA_DIST += $(pkgtests_DATA)

BUILD_SH_TP = \
	echo "Creating $${dst}"; \
	echo "\#! $(bindir)/atf-sh" >$${dst}; \
	cat $${src} >>$${dst}; \
	chmod +x $${dst}

#
# Custom targets.
#

.PHONY: clean-all
clean-all:
	MTN="$(MTN)" $(SH) $(srcdir)/admin/clean-all.sh

.PHONY: release
release:
	$(SH) $(srcdir)/admin/release.sh $(PACKAGE_VERSION) $(DIST_ARCHIVES)

.PHONY: release-test
release-test:
	XML_CATALOG_FILE="$(XML_CATALOG_FILE)" $(SH) \
		$(srcdir)/admin/release-test.sh $(DIST_ARCHIVES)

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
