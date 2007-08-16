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

create_helper()
{
    cat >helper.sh <<EOF
tc_head()
{
    atf_set "descr" "A helper test case"
}
tc_body()
{
EOF
    cat >>helper.sh
    cat >>helper.sh <<EOF
}

atf_init_test_cases()
{
    atf_add_test_case tc
}
EOF
    atf-compile -o helper helper.sh
    rm -f helper.sh
}

create_pass_helper()
{
    create_helper <<EOF
:
EOF
    mv helper ${1}
}

create_fail_helper()
{
    create_helper <<EOF
atf_fail "Foobar"
EOF
    mv helper ${1}
}

create_helpers()
{
    mkdir dir1
    create_pass_helper dir1/tp1
    create_fail_helper dir1/tp2
    create_pass_helper tp3
    create_fail_helper tp4

    cat >Atffile <<EOF
Content-Type: application/X-atf-atffile; version="0"

test-suite: atf

dir1
tp3
tp4
EOF

    cat >dir1/Atffile <<EOF
Content-Type: application/X-atf-atffile; version="0"

test-suite: atf

tp1
tp2
EOF
}

default_head()
{
    atf_set "descr" "Checks that the default output uses the ticker" \
                    "format"
    atf_set "require.progs" "atf-compile" # XXX
}
default_body()
{
    create_helpers
    atf-run >tps.out 2>/dev/null

    # Check that the default output uses the ticker format.
    atf_check 'atf-report <tps.out' 0 stdout null
    atf_check 'grep "test cases" stdout' 0 ignore null
    atf_check 'grep "Failed test cases" stdout' 0 ignore null
    atf_check 'grep "Summary for" stdout' 0 ignore null
}

oflag_head()
{
    atf_set "descr" "Checks that the -o flag works"
    atf_set "require.progs" "atf-compile" # XXX
}
oflag_body()
{
    create_helpers
    atf-run >tps.out 2>/dev/null

    cat >expcsv <<EOF
dir1/tp1, tc, passed
dir1/tp2, tc, failed, Foobar
tp3, tc, passed
tp4, tc, failed, Foobar
EOF

    # Check that changing the stdout output works.
    cp expcsv expout
    atf_check 'atf-report -o csv:- <tps.out' 0 expout null
    rm -f expout

    # Check that sending the output to a file does not write to stdout.
    atf_check 'atf-report -o csv:fmt.out <tps.out' 0 null null
    atf_check 'cmp -s expcsv fmt.out' 0 null null
    rm -f fmt.out

    # Check that defining two outputs using the same format works.
    atf_check 'atf-report -o csv:fmt.out -o csv:fmt2.out <tps.out' \
              0 null null
    atf_check 'cmp -s expcsv fmt.out' 0 null null
    atf_check 'cmp -s fmt.out fmt2.out' 0 null null
    rm -f fmt.out fmt2.out

    # Check that defining two outputs using different formats works.
    atf_check 'atf-report -o csv:fmt.out -o ticker:fmt2.out <tps.out' \
              0 null null
    atf_check 'cmp -s expcsv fmt.out' 0 null null
    atf_check 'cmp -s fmt.out fmt2.out' 1 null null
    atf_check 'grep "test cases" fmt2.out' 0 ignore null
    atf_check 'grep "Failed test cases" fmt2.out' 0 ignore null
    atf_check 'grep "Summary for" fmt2.out' 0 ignore null
    rm -f fmt.out fmt2.out

    # Check that defining two outputs over the same file does not work.
    atf_check 'atf-report -o csv:fmt.out -o ticker:fmt.out <tps.out' \
              1 null stderr
    atf_check 'grep "more than once" stderr' 0 ignore null
    rm -f fmt.out

    # Check that defining two outputs over stdout (but using different
    # paths) does not work.
    atf_check 'atf-report -o csv:- -o ticker:/dev/stdout <tps.out' \
              1 null stderr
    atf_check 'grep "more than once" stderr' 0 ignore null
    rm -f fmt.out
}

atf_init_test_cases()
{
    atf_add_test_case default
    atf_add_test_case oflag
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
