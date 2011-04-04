#
# Automated Testing Framework (atf)
#
# Copyright (c) 2011 The NetBSD Foundation, Inc.
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

# ATF_CHECK_SH([version-spec])
#
# Checks if atf-sh is present.  If version-spec is provided, ensures that
# the installed version of atf-sh matches the required version.  This
# argument must be something like '>= 0.14' and accepts any version
# specification supported by pkg-config.
#
# Defines and substitutes ATF_SH with the full path to the atf-sh interpreter.
AC_DEFUN([ATF_CHECK_SH], [
    spec="atf-sh[]m4_default_nblank([ $1], [])"
    AC_MSG_CHECKING([for ${spec}])
    PKG_CHECK_EXISTS([${spec}], [found=yes], [found=no])
    if test "${found}" = no; then
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([${spec} not found but it is required])
    fi
    ATF_SH="$(${PKG_CONFIG} --variable=interpreter atf-sh)"
    AC_SUBST([ATF_SH], [${ATF_SH}])
    AC_MSG_RESULT([${ATF_SH}])
])
