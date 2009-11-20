#! /bin/sh

set -e

./admin/generate-makefile.sh Makefile.am.m4 Makefile.am
autoreconf -is

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
