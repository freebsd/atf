#! /bin/sh
#
# Automated Testing Framework (atf)
#
# Copyright (c) 2010 The NetBSD Foundation, Inc.
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

#
# A utility to prepare the files that make up a release.
#

set -e

: ${GPG:=gpg}
: ${SHA1:=digest sha1}

Prog_Name=${0##*/}

# err message
#
err() {
    echo "${Prog_Name}: ${@}" 1>&2
    exit 1
}

# info message
#
info() {
    echo "${Prog_Name}: ${@}"
}

# checksum_release dir distfile cksumfile
#
checksum_release() {
    local dir="${1}"; shift
    local distfile="${1}"; shift
    local cksumfile="${1}"; shift

    info "Checksumming ${distfile} into ${cksumfile}"
    cd "${dir}"
    ${SHA1} "${distfile}" >"${cksumfile}"
    cd -
}

# sign_release dir distfile sigfile
#
sign_release() {
    local dir="${1}"; shift
    local distfile="${1}"; shift
    local sigfile="${1}"; shift

    info "Signing ${distfile} into ${sigfile}"
    cd "${dir}"
    ${GPG} --detach-sign --armor --output="${sigfile}" "${distfile}"
    cd -
}

# main version distfiles ...
#
main() {
    [ ${#} -ge 2 ] || err "Syntax error: must specify version and distfiles"
    local version="${1}"; shift

    local dir="${version}"
    if [ -d "${dir}" ]; then
        info "Moving stale ${dir} to ${dir}.old"
        rm -rf "${dir}.old"
        mv "${dir}" "${dir}.old"
    fi
    mkdir "${dir}"

    for distname in "${@}"; do
        [ -f "${distname}" ] || err "Cannot find ${distname}; run make dist?"
        cp "${distname}" "${dir}"
        checksum_release "${dir}" "${distname}" "${distname}.cksums"
        sign_release "${dir}" "${distname}" "${distname}.asc"
    done

    info "Successful release stored in ${dir}/"
    info "Do not forget to tag this revision in the repository!"
}

main "${@}"

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
