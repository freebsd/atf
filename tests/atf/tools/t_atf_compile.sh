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

# TODO: This test program is probably incomplete.  It was first added
# to demonstrate the utility of the atf_check function.  Inspect its
# current status and complete it.

tc_includes_head()
{
    atf_set "descr" "Tests that the resulting file includes the correct" \
                    "shell subroutines"
}
tc_includes_body()
{
    cat >tp_test.sh <<EOF
# This is a sample test program.
EOF
    atf_check 'atf-compile -o tp_test tp_test.sh' 0 null null
    atf_check 'grep ^\..*/atf.init.subr tp_test' 1 null null
    atf_check 'grep ^\..*/atf.header.subr tp_test' 0 ignore null
    atf_check 'grep ^\..*/atf.footer.subr tp_test' 0 ignore null
}

tc_oflag_head()
{
    atf_set "descr" "Tests that the -o flag works"
}
tc_oflag_body()
{
    atf_check 'touch tp_foo.sh' 0 null null
    atf_check 'atf-compile tp_foo.sh' 0 stdout null
    atf_check 'test -f tp_foo' 1 null null
    atf_check 'atf-compile -o tp_foo tp_foo.sh' 0 null null
    atf_check 'test -f tp_foo' 0 null null
    atf_check 'cmp stdout tp_foo' 0 ignore null
}

atf_init_test_cases()
{
    atf_add_test_case tc_oflag
    atf_add_test_case tc_includes
}
