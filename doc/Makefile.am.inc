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

man_MANS += doc/atf.7
CLEANFILES += doc/atf.7
EXTRA_DIST += doc/atf.7.in

dist_man_MANS += doc/atf-formats.5 \
                 doc/atf-test-case.4 \
                 doc/atf-test-program.1

doc/atf.7: $(srcdir)/doc/atf.7.in
	test -d doc || mkdir -p doc
	sed -e 's,__DOCDIR__,$(docdir),g' \
	    <$(srcdir)/doc/atf.7.in >doc/atf.7.tmp
	mv doc/atf.7.tmp doc/atf.7

_STANDALONE_XSLT = doc/standalone/sdocbook.xsl

EXTRA_DIST += doc/build-xml.sh
EXTRA_DIST += doc/standalone/standalone.css
EXTRA_DIST += $(_STANDALONE_XSLT)

BUILD_XML_ENV = DOC_BUILD=$(DOC_BUILD) \
                LINKS=$(LINKS) \
                TIDY=$(TIDY) \
                XML_CATALOG_FILE=$(XML_CATALOG_FILE) \
                XMLLINT=$(XMLLINT) \
                XSLTPROC=$(XSLTPROC)

BUILD_HTML = \
	$(BUILD_XML_ENV) $(ATF_SHELL) doc/build-xml.sh \
	    $(srcdir)/doc/$${name}.xml $(srcdir)/$(_STANDALONE_XSLT) \
	    html:$(srcdir)/doc/standalone/$${name}.html

BUILD_TXT = \
	$(BUILD_XML_ENV) $(ATF_SHELL) doc/build-xml.sh \
	    $(srcdir)/doc/$${name}.xml $(srcdir)/$(_STANDALONE_XSLT) \
	    txt:$(srcdir)/doc/text/$${name}.txt

DOC_DEPS = doc/build-xml.sh $(_STANDALONE_XSLT)

EXTRA_DIST += doc/authors.xml
EXTRA_DIST += doc/standalone/authors.html
noinst_DATA += doc/standalone/authors.html
@target_srcdir@doc/standalone/authors.html: $(DOC_DEPS) \
                                            $(srcdir)/doc/authors.xml
	name=authors; $(BUILD_HTML)
EXTRA_DIST += doc/text/authors.txt
noinst_DATA += doc/text/authors.txt
@target_srcdir@doc/text/authors.txt: $(DOC_DEPS) \
                                     $(srcdir)/doc/authors.xml
	name=authors; $(BUILD_TXT)

EXTRA_DIST += doc/copying.xml
EXTRA_DIST += doc/standalone/copying.html
noinst_DATA += doc/standalone/copying.html
@target_srcdir@doc/standalone/copying.html: $(DOC_DEPS) \
                                            $(srcdir)/doc/copying.xml
	name=copying; $(BUILD_HTML)
EXTRA_DIST += doc/text/copying.txt
noinst_DATA += doc/text/copying.txt
@target_srcdir@doc/text/copying.txt: $(DOC_DEPS) \
                                     $(srcdir)/doc/copying.xml
	name=copying; $(BUILD_TXT)

EXTRA_DIST += doc/install.xml
EXTRA_DIST += doc/standalone/install.html
noinst_DATA += doc/standalone/install.html
@target_srcdir@doc/standalone/install.html: $(DOC_DEPS) \
                                            $(srcdir)/doc/install.xml
	name=install; $(BUILD_HTML)
EXTRA_DIST += doc/text/install.txt
noinst_DATA += doc/text/install.txt
@target_srcdir@doc/text/install.txt: $(DOC_DEPS) \
                                     $(srcdir)/doc/install.xml
	name=install; $(BUILD_TXT)

EXTRA_DIST += doc/news.xml
EXTRA_DIST += doc/standalone/news.html
noinst_DATA += doc/standalone/news.html
@target_srcdir@doc/standalone/news.html: $(DOC_DEPS) \
                                         $(srcdir)/doc/news.xml
	name=news; $(BUILD_HTML)
EXTRA_DIST += doc/text/news.txt
noinst_DATA += doc/text/news.txt
@target_srcdir@doc/text/news.txt: $(DOC_DEPS) \
                                  $(srcdir)/doc/news.xml
	name=news; $(BUILD_TXT)

EXTRA_DIST += doc/readme.xml
EXTRA_DIST += doc/standalone/readme.html
noinst_DATA += doc/standalone/readme.html
@target_srcdir@doc/standalone/readme.html: $(DOC_DEPS) \
                                           $(srcdir)/doc/readme.xml
	name=readme; $(BUILD_HTML)
EXTRA_DIST += doc/text/readme.txt
noinst_DATA += doc/text/readme.txt
@target_srcdir@doc/text/readme.txt: $(DOC_DEPS) \
                                    $(srcdir)/doc/readme.xml
	name=readme; $(BUILD_TXT)

EXTRA_DIST += doc/specification.xml
EXTRA_DIST += doc/standalone/specification.html
noinst_DATA += doc/standalone/specification.html
@target_srcdir@doc/standalone/specification.html: $(DOC_DEPS) \
    $(srcdir)/doc/specification.xml
	name=specification; $(BUILD_HTML)
EXTRA_DIST += doc/text/specification.txt
noinst_DATA += doc/text/specification.txt
@target_srcdir@doc/text/specification.txt: $(DOC_DEPS) \
                                           $(srcdir)/doc/specification.xml
	name=specification; $(BUILD_TXT)

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
