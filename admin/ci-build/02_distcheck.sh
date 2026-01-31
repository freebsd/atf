#!/bin/sh
#
# Step 2: run `make distcheck`.
#
# `make distcheck` builds the tests, runs them, and subsequently performs a
# style check on the code.
#
# SPDX-License-Identifier: BSD-2-Clause

set -eux

: "${AS_ROOT=no}"
: "${CC=cc}"
: "${CXX=c++}"
: "${EXTRA_DISTCHECK_CONFIGURE_ARGS=}"

NPROC=$(nproc 2>/dev/null || getconf NPROCESSORS_ONLN 2>/dev/null || echo 1)

f=
f="${f} ATF_BUILD_CC='${CC}'"
f="${f} ATF_BUILD_CXX='${CXX}'"
# Is this being run in a git clone, or with a release artifact?
if git rev-parse --is-inside-work-tree; then
    f="${f} --enable-developer"
fi
if [ -n "${EXTRA_DISTCHECK_CONFIGURE_ARGS:-}" ]; then
    f="${f} ${EXTRA_DISTCHECK_CONFIGURE_ARGS}"
fi

kyua_conf="$(realpath "$(mktemp kyuaconf-XXXXXXXX)")"
trap 'rm -f "${kyua_conf}"' EXIT INT TERM

sudo=

cat >"${kyua_conf}" <<EOF
syntax(2)

unprivileged_user = 'nobody'
EOF

if [ "${AS_ROOT}" = yes ]; then
    sudo="sudo -H"
fi
${sudo} env PATH="${PATH}" make distcheck -j"${NPROC}" \
    DISTCHECK_CONFIGURE_FLAGS="${f}" \
    KYUA_TEST_CONFIG_FILE="${kyua_conf}"

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
