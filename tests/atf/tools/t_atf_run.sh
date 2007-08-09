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

separator_head()
{
    atf_set "descr" "Ensures that the test case's error message is " \
                    "properly handled if it includes the separator " \
                    "used to format the output"
}
separator_body()
{
    cat >tc.sh <<EOF
#! /bin/sh
# Note that the following look like four fields, but in reality they are
# three.  That's what we are checking for here.
echo 'Content-Type: application/X-atf-tcs; version="0"' >&9
echo '' >&9
echo "tc1, failed, This test failed, second part" >&9
echo "tc2, skipped, This test was skipped, second part" >&9
exit 1
EOF
    chmod +x tc.sh
    cat >Atffile <<EOF
Content-Type: application/X-atf-atffile; version="0"

tc.sh
EOF
    atf_check 'atf-run' 1 stdout stderr
    atf_check 'grep -i "failed.*This test failed, second part" stdout' \
              0 ignore null
    atf_check 'grep -i "skipped.*This test was skipped, second part" stdout' \
              0 ignore null
}

atf_init_test_cases()
{
    atf_add_test_case separator
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
