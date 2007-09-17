#
# Automated Testing Framework (atf)
#
# Copyright (c) 2007 The NetBSD Foundation, Inc.
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

run_tests()
{
    h=${1} what=${2} status=${3} code=${4}

    atf_check "${h} -s ${srcdir} -r3 -v 'cleanup=no' \
               -v 'tmpfile=$(pwd)/tmpfile' \
               cleanup_${what} 3>resout" ${code} ignore ignore
    atf_check "grep 'cleanup_${what}, ${status}' resout" 0 ignore null
    atf_check "test -f tmpfile" 0 null null
    atf_check "rm tmpfile" 0 null null

    atf_check "${h} -s ${srcdir} -r3 -v 'cleanup=yes' \
               -v 'tmpfile=$(pwd)/tmpfile' \
               cleanup_${what} 3>resout" ${code} ignore ignore
    atf_check "grep 'cleanup_${what}, ${status}' resout" 0 ignore null
    atf_check "test -f tmpfile" 1 null null
}

atf_test_case main
main_head()
{
    atf_set "descr" "Tests that the cleanup hook works"
}
main_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        run_tests ${h} pass passed 0
        run_tests ${h} fail failed 1
        run_tests ${h} skip skipped 0
    done
}

atf_init_test_cases()
{
    atf_add_test_case main
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
