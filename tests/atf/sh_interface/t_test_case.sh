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

default_status_head()
{
    atf_set "descr" "Verifies that test cases get the correct default" \
                    "status if they did not provide any"
}
default_status_body()
{
    cat >helper.sh <<EOF
pass_true_head() {
    atf_set "descr" "Helper test case"
}
pass_true_body() {
    true
}

pass_false_head() {
    atf_set "descr" "Helper test case"
}
pass_false_body() {
    false
}

fail_head() {
    atf_set "descr" "Helper test case"
}
fail_body() {
    echo "An error" 1>&2
    exit 1
}

atf_init_test_cases() {
    atf_add_test_case pass_true
    atf_add_test_case pass_false
    atf_add_test_case fail
}
EOF
    atf-compile -o helper helper.sh

    atf_check './helper -r3 pass_true 3>resout' 0 ignore ignore
    atf_check './helper -r3 pass_false 3>resout' 0 ignore ignore
    atf_check './helper -r3 fail 3>resout' 1 ignore stderr
    atf_check 'grep "An error" stderr' 0 ignore null
}

atf_init_test_cases()
{
    atf_add_test_case default_status
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
