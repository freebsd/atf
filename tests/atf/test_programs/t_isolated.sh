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

default_head()
{
    atf_set "descr" "Tests that the 'isolated' test property works" \
                    "when the user does not override the work directory"
}
default_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(cd $(atf-config -t atf_workdir) && pwd -P)

    for h in ${h_cpp} ${h_sh}; do
        atf_check "ISOLATED=no PATHFILE=$(pwd)/expout ${h} \
                  -s ${srcdir} isolated_path" 0 ignore ignore
        atf_check "pwd -P" 0 expout null

        atf_check "ISOLATED=yes PATHFILE=$(pwd)/path ${h} \
                  -s ${srcdir} isolated_path" 0 ignore ignore
        atf_check "grep '^${tmpdir}/atf.' <path" 0 ignore ignore
    done
}

tmpdir_head()
{
    atf_set "descr" "Tests that the 'isolated' test property works" \
                    "when the user overrides the work directory through" \
                    "the ATF_WORKDIR environment variable"
}
tmpdir_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(pwd -P)/workdir
    mkdir ${tmpdir}

    for h in ${h_cpp} ${h_sh}; do
        atf_check "ISOLATED=no PATHFILE=$(pwd)/expout \
                  ATF_WORKDIR=${tmpdir} ${h} \
                  -s ${srcdir} isolated_path" 0 ignore ignore
        atf_check "pwd -P" 0 expout null

        atf_check "ISOLATED=yes PATHFILE=$(pwd)/path \
                  ATF_WORKDIR=${tmpdir} ${h} \
                  -s ${srcdir} isolated_path" 0 ignore ignore
        atf_check "grep '^${tmpdir}/atf.' <path" 0 ignore ignore
    done
}

wflag_head()
{
    atf_set "descr" "Tests that the 'isolated' test property works" \
                    "when the user overrides the work directory through" \
                    "the test program's -w flag"
}
wflag_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(pwd -P)/workdir
    mkdir ${tmpdir}

    for h in ${h_cpp} ${h_sh}; do
        atf_check "ISOLATED=no PATHFILE=$(pwd)/expout ${h} \
                  -s ${srcdir} -w ${tmpdir} isolated_path" 0 ignore ignore
        atf_check "pwd -P" 0 expout null

        atf_check "ISOLATED=yes PATHFILE=$(pwd)/path ${h} \
                  -s ${srcdir} -w ${tmpdir} isolated_path" 0 ignore ignore
        atf_check "grep '^${tmpdir}/atf.' <path" 0 ignore null
    done
}

cleanup_head()
{
    atf_set "descr" "Tests that the directories used to isolate test" \
                    "cases are properly cleaned up once the test case" \
                    "exits"
}
cleanup_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(pwd -P)/workdir
    mkdir ${tmpdir}

    for h in ${h_cpp} ${h_sh}; do
        # First try to clean a work directory that, supposedly, does not
        # have any subdirectories.
        atf_check "ISOLATED=yes PATHFILE=$(pwd)/path ${h} \
                  -s ${srcdir} -w ${tmpdir} isolated_path" 0 ignore ignore
        atf_check "test -d $(cat path)" 1 null null

        # Now do the same but with a work directory that has subdirectories.
        # The program will have to recurse into them to clean them all.
        atf_check "PATHFILE=$(pwd)/path ${h} \
                  -s ${srcdir} -w ${tmpdir} isolated_cleanup" 0 ignore ignore
        atf_check "test -d $(cat path)" 1 null null
    done
}

atf_init_test_cases()
{
    atf_add_test_case default
    atf_add_test_case tmpdir
    atf_add_test_case wflag
    atf_add_test_case cleanup
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
