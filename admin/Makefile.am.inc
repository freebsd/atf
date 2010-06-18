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

dist-hook: check-install
.PHONY: check-install
check-install: $(srcdir)/INSTALL
	$(srcdir)/admin/check-install.sh $(srcdir)/INSTALL

dist-hook: check-style
.PHONY: check-style
check-style:
	$(srcdir)/admin/check-style.sh

EXTRA_DIST += admin/check-install.sh \
              admin/check-style-common.awk \
              admin/check-style-c.awk \
              admin/check-style-cpp.awk \
              admin/check-style-man.awk \
              admin/check-style-shell.awk \
              admin/check-style.sh \
              admin/choose-revision.sh \
              admin/generate-revision.sh \
              admin/generate-revision-dist.sh

.PHONY: admin/revision.h
admin/revision.h:
	@test -d admin || mkdir -p admin
	@$(top_srcdir)/admin/generate-revision.sh \
	    -f h -m "$(MTN)" -r $(top_srcdir) -o admin/revision.h \
	    -v $(PACKAGE_VERSION)
BUILT_SOURCES += admin/revision.h
CLEANFILES += admin/revision.h

dist-hook: $(srcdir)/admin/revision-dist.h
$(srcdir)/admin/revision-dist.h: admin/revision.h
	@$(top_srcdir)/admin/generate-revision-dist.sh \
	    -f h -i admin/revision.h -o $(srcdir)/admin/revision-dist.h
BUILT_SOURCES += $(srcdir)/admin/revision-dist.h
EXTRA_DIST += admin/revision-dist.h

# vim: syntax=make:noexpandtab:shiftwidth=8:softtabstop=8
