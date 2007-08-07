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

common_tests() {
    where=${1}
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        # Check absolute paths.
        atf_check "PROGS='/bin/cp' \
                   ${h} -s ${srcdir} -r3 require_progs_${where} \
                   3>resout" 0 ignore ignore
        atf_check 'grep "require_progs_${where}, passed" resout' \
                  0 ignore null
        atf_check "PROGS='/bin/___non-existent___' \
                   ${h} -s ${srcdir} -r3 require_progs_${where} \
                   3>resout" 0 ignore ignore
        atf_check 'grep "require_progs_${where}, skipped" resout' \
                  0 ignore null

        # Relative paths are not allowed.
        atf_check "PROGS='bin/cp' \
                   ${h} -s ${srcdir} -r3 require_progs_${where} \
                   3>resout" 1 ignore stderr
        atf_check 'grep "require_progs_${where}, failed" resout' \
                  0 ignore null

        # Check plain file names, searching them in the PATH.
        atf_check "PROGS='cp' \
                   ${h} -s ${srcdir} -r3 require_progs_${where} \
                   3>resout" 0 ignore ignore
        atf_check 'grep "require_progs_${where}, passed" resout' \
                  0 ignore null
        atf_check "PROGS='___non-existent___' \
                   ${h} -s ${srcdir} -r3 require_progs_${where} \
                   3>resout" 0 ignore ignore
        atf_check 'grep "require_progs_${where}, skipped" resout' \
                  0 ignore null
    done
}

require_prog_func_head()
{
    atf_set "descr" "Tests that atf_require_prog works"
}
require_prog_func_body()
{
    common_tests body
}

require_progs_header_head()
{
    atf_set "descr" "Tests that require.progs works"
}
require_progs_header_body()
{
    common_tests head

    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        # Check a couple of absolute path names.  The second must make
        # the check fail.
        atf_check "PROGS='/bin/cp /bin/___non-existent___' \
                   ${h} -s ${srcdir} -r3 require_progs_head \
                   3>resout" 0 ignore ignore
        atf_check 'grep "skipped.*non-existent" resout' 0 ignore null

        # Check a couple of absolute path names.  Both have to be found.
        atf_check "PROGS='/bin/cp /bin/ls' \
                   ${h} -s ${srcdir} -r3 require_progs_head \
                   3>resout" 0 ignore ignore
        atf_check 'grep "passed" resout' 0 ignore null

        # Check an absolute path name and a relative one.  The second must
        # make the check fail.
        atf_check "PROGS='/bin/cp bin/cp' \
                   ${h} -s ${srcdir} -r3 require_progs_head \
                   3>resout" 1 ignore stderr
        atf_check 'grep "failed" resout' 0 ignore null

        # Check an absolute path name and a plain one.  Both have to be
        # found.
        atf_check "PROGS='/bin/cp ls' \
                   ${h} -s ${srcdir} -r3 require_progs_head \
                   3>resout" 0 ignore ignore
        atf_check 'grep "passed" resout' 0 ignore null

        # Check an absolute path name and a plain one.  The second must
        # make the check fail.
        atf_check "PROGS='/bin/cp ___non-existent___' \
                   ${h} -s ${srcdir} -r3 require_progs_head \
                   3>resout" 0 ignore ignore
        atf_check 'grep "skipped,.*non-existent" resout' 0 ignore null
    done
}

atf_init_test_cases()
{
    atf_add_test_case require_prog_func
    atf_add_test_case require_progs_header
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
