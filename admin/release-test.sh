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
# A utility to validate a release file.
#

set -e

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

# run_test distpath make parallelism configure_args
#
run_test() {
    local distpath="${1}"; shift
    local make="${1}"; shift
    local jflag="${1}"; shift

    echo "Make tool: ${make}"
    echo "Parallelism: ${jflag}"
    echo "Configure args: ${@}"
    echo

    tar xzvf "${distpath}"
    cd atf-[0-9]*

    ./configure --prefix $(pwd)/local "${@}" || return 1

    if [ ${jflag} -gt 1 ]; then
        ${make} -j${jflag} || return 1
    else
        ${make} || return 1
    fi
    ${make} install || return 1

    $(pwd)/local/bin/atf-version >version.txt || return 1
    if grep 'Built from a distribution file' version.txt >/dev/null; then
        :
    else
        echo "ERROR: atf-version reports sources from repository"
        return 1
    fi

    ${make} installcheck || return 1
    sudo ${make} installcheck || return 1
    sudo rm installcheck.*
    rm -rf local

    ${make} distcheck || return 1

    ${make} distclean || return 1

    cd -
    rm -rf atf-[0-9]*
}

# one_test logfile (same arguments as run_test)
#
one_test() {
    local logfile="${1}"; shift

    info "Test: ${*}; log is ${logfile##*/}"
    if run_test "${@}" >"${logfile}" 2>&1; then
        true
    else
        info "Release test failed: ${*}"
        false
    fi
}

# require_package package
#
require_package() {
    local package="${1}"; shift

    if pkg_info -E "${package}" >/dev/null; then
        info "Required package ${package} found"
    else
        err "Required package ${package} not installed"
    fi
}

# validate_sudo
#
validate_sudo() {
    info "Refreshing sudo credentials; enter your password if needed"
    sudo -v

    info "Validating sudo settings"

    local line="$(sudo -l | grep timestamp_timeout)"
    [ -n "${line}" ] || err "timestamp_timeout not defined"

    local timestamp_timeout=$(printf '%s' "${line}" | cut -d = -f 2)
    [ "${timestamp_timeout}" -ge 60 ] || \
        err "timestamp_timeout too low; release process may stop in the middle"
}

# main distfile
#
main() {
    [ ${#} -eq 1 ] || err "Syntax error: must specify a distfile"
    local distfile="${1}"; shift

    [ -f "${distfile}" ] || err "Cannot open distfile ${distfile}"
    local distpath=
    case "${distfile}" in
    /*) distpath="${distfile}" ;;
    *) distpath="$(pwd)/${distfile}" ;;
    esac

    require_package pkg-config
    require_package sudo

    validate_sudo

    local logdir="$(pwd)/release-test"
    mkdir -p "${logdir}"
    info "Saving log files in ${logdir}"

    local dir="$(mktemp -d /tmp/atf.XXXXXX)"
    trap "rm -rf '${dir}'" HUP INT QUIT TERM
    cd "${dir}"

    local count=0
    local failed=no
    local make=
    for make in make gmake; do
        for parallelism in 1 4; do
            local logfile

            count=$((count + 1))
            logfile="${logdir}/${count}.log"
            one_test "${logfile}" "${distpath}" "${make}" "${parallelism}" \
                ATF_SHELL=/bin/sh || failed=yes
        done
    done

    rm -rf "${dir}"

    if [ "${failed}" = yes ]; then
        err "Some tests failed; please investigate the logs"
    else
        info "Successful release-test"
        true
    fi
}

main "${@}"

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
