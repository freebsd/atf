#! /bin/sh

set -e -x

autoreconf -i -s
./configure

if [ "${AS_ROOT:-no}" = yes ]; then
    sudo make distcheck
else
    make distcheck
fi
