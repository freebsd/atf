#!/bin/sh
#
# Step 1: run autoconf/configure so the remaining of the build/test steps can
# be completed.
#
# Splitting off this step allows any configure issues to be found quickly and
# triaged more effectively.
#
# SPDX-License-Identifier: BSD-2-Clause

autoreconf_args="-isv"
if [ -d /usr/local/share/aclocal ]; then
    autoreconf_args="${autoreconf_args} -I/usr/local/share/aclocal"
fi
# shellcheck disable=SC2086
autoreconf ${autoreconf_args}

if ! ./configure; then
    cat config.log || true
    exit 1
fi

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
