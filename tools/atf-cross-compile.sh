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

if [ -z "${ATF_PKGDATADIR}" ]; then
    echo "atf-cross-compile: ATF_PKGDATADIR is undefined" 1>&2
    exit 1
fi

if [ ! -f "${ATF_PKGDATADIR}/atf.init.subr" ]; then
    echo "atf-cross-compile: ATF_PKGDATADIR is not valid" 1>&2
    exit 1
fi

if [ -z "${ATF_SHELL}" ]; then
    echo "atf-cross-compile: ATF_SHELL is undefined" 1>&2
    exit 1
fi

if [ ${#} -lt 1 ]; then
    echo "atf-cross-compile: no -o option specified" 1>&2
    exit 1
fi

if [ ${1} != '-o' ]; then
    echo "atf-cross-compile: no -o option specified" 1>&2
    exit 1
fi
if [ ${#} -lt 2 ]; then
    echo "atf-cross-compile: no source file specified" 1>&2
    exit 1
fi

if [ ${#} -lt 3 ]; then
    echo "atf-cross-compile: no target file specified" 1>&2
    exit 1
fi

tfile=${2}
sfile=${3}

if [ ! -f "${sfile}" ]; then
    echo "atf-cross-compile: source file does not exist" 1>&2
    exit 1
fi

echo "#! ${ATF_SHELL}" >${tfile}
cat ${ATF_PKGDATADIR}/atf.init.subr >>${tfile}
echo >>${tfile}
echo '. ${Atf_Pkgdatadir}/atf.header.subr' >>${tfile}
echo >>${tfile}
cat <${sfile} >>${tfile}
echo '. ${Atf_Pkgdatadir}/atf.footer.subr' >>${tfile}
echo >>${tfile}
echo "main \"\${@}\"" >>${tfile}

chmod +x ${tfile}

exit 0

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
