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

count_lines()
{
    wc -l $1 | awk '{ print $1 }'
}

root_head()
{
    atf_set "descr" "Tests that 'require.user=root' works"
}
root_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 require_user_root \
            3>resout" 0 ignore ignore
        if [ $(id -u) -eq 0 ]; then
            atf_check 'grep "require_user_root, passed" resout' \
                0 ignore null
        else
            atf_check 'grep "require_user_root, skipped" resout' \
                0 ignore null
        fi
    done
}

unprivileged_head()
{
    atf_set "descr" "Tests that 'require.user=unprivileged' works"
}
unprivileged_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 require_user_unprivileged \
            3>resout" 0 ignore ignore
        if [ $(id -u) -eq 0 ]; then
            atf_check 'grep "require_user_unprivileged, skipped" resout' \
                0 ignore null
        else
            atf_check 'grep "require_user_unprivileged, passed" resout' \
                0 ignore null
        fi
    done
}

multiple_head()
{
    atf_set "descr" "Tests that multiple skip results raised by the" \
                    "handling of require.user do not abort the program" \
                    "prematurely"
}
multiple_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 'require_user*' 3>resout" \
            0 ignore ignore
        grep ", skipped, " resout >skips
        [ $(count_lines skips) -lt 2 ] && \
            atf_fail "Test program aborted prematurely"
        [ $(count_lines skips) -gt 2 ] && \
            atf_fail "Test program returned more skips than expected"
    done
    atf_pass # XXX Bogus!
}

atf_init_test_cases()
{
    atf_add_test_case root
    atf_add_test_case unprivileged
    atf_add_test_case multiple
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
