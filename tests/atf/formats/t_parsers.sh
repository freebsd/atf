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
    type=${1}; shift
    sd=$(atf_get_srcdir)
    while [ ${#} -gt 0 ]; do
        if [ -f ${sd}/${1}.expout ]; then
            cp ${sd}/${1}.expout expout
        else
            touch expout
        fi

        if [ -f ${sd}/${1}.experr ]; then
            cp ${sd}/${1}.experr experr
        else
            touch experr
        fi

        if [ -f ${sd}/${1}.outin ]; then
            cp ${sd}/${1}.outin outin
        else
            touch outin
        fi

        if [ -f ${sd}/${1}.errin ]; then
            cp ${sd}/${1}.errin errin
        else
            touch errin
        fi

        atf_check "${sd}/h_parser ${type} ${sd}/${1} outin errin" \
                  0 expout experr
        rm -f expout experr

        shift
    done
}

atffile_head()
{
    atf_set "descr" "Verifies the application/X-atf-atffile parser"
}
atffile_body()
{
    run_tests application/X-atf-atffile \
        d_atffile_1 \
        d_atffile_2 \
        d_atffile_3 \
        d_atffile_4 \
        d_atffile_5 \
        d_atffile_6
}

config_head()
{
    atf_set "descr" "Verifies the application/X-atf-config parser"
}
config_body()
{
    run_tests application/X-atf-config \
        d_config_1 \
        d_config_2 \
        d_config_3 \
        d_config_4
}

tcs_head()
{
    atf_set "descr" "Verifies the application/X-atf-tcs parser"
}
tcs_body()
{
    run_tests application/X-atf-tcs \
        d_tcs_1 \
        d_tcs_2 \
        d_tcs_3 \
        d_tcs_4 \
        d_tcs_5
}

tps_head()
{
    atf_set "descr" "Verifies the application/X-atf-tps parser"
}
tps_body()
{
    run_tests application/X-atf-tps \
        d_tps_1 \
        d_tps_2 \
        d_tps_3
}

atf_init_test_cases()
{
    atf_add_test_case atffile
    atf_add_test_case config
    atf_add_test_case tcs
    atf_add_test_case tps
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
