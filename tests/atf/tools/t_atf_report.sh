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

create_helpers()
{
    mkdir dir1
    cp $(atf_get_srcdir)/h_pass dir1/tp1
    cp $(atf_get_srcdir)/h_fail dir1/tp2
    cp $(atf_get_srcdir)/h_pass tp3
    cp $(atf_get_srcdir)/h_fail tp4

    cat >Atffile <<EOF
Content-Type: application/X-atf-atffile; version="1"

prop: test-suite = atf

tp: dir1
tp: tp3
tp: tp4
EOF

    cat >dir1/Atffile <<EOF
Content-Type: application/X-atf-atffile; version="1"

prop: test-suite = atf

tp: tp1
tp: tp2
EOF
}

atf_test_case default
default_head()
{
    atf_set "descr" "Checks that the default output uses the ticker" \
                    "format"
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

atf_test_case oflag
oflag_head()
{
    atf_set "descr" "Checks that the -o flag works"
}
oflag_body()
{
    create_helpers
    atf-run >tps.out 2>/dev/null

    cat >expcsv <<EOF
tc, dir1/tp1, main, passed
tp, dir1/tp1, passed
tc, dir1/tp2, main, failed, This always fails
tp, dir1/tp2, failed
tc, tp3, main, passed
tp, tp3, passed
tc, tp4, main, failed, This always fails
tp, tp4, failed
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
