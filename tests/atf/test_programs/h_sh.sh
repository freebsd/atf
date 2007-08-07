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

env_undef_head()
{
    atf_set "descr" "Helper test case for the t_mangle_fds test program"
}
env_undef_body()
{
    [ -n "${LC_COLLATE}" ] && atf_fail "LC_COLLATE is defined"
    [ -n "${TZ}" ] && atf_fail "TZ is defined"
}

fork_mangle_fds_head()
{
    atf_set "descr" "Helper test case for the t_mangle_fds test program"

    if [ -z "${ISOLATED}" ]; then
        atf_set "isolated" "invalid-value"
    else
        atf_set "isolated" "${ISOLATED}"
    fi
}
fork_mangle_fds_body()
{
    test -z "${RESFD}" && atf_fail "RESFD not defined"
    touch in
    0<&in
    1>&out
    2>&err
    ${RESFD}>&res
}

isolated_path_head()
{
    atf_set "descr" "Helper test case for the t_isolated test program"

    if [ -z "${ISOLATED}" ]; then
        atf_set "isolated" "invalid-value"
    else
        atf_set "isolated" "${ISOLATED}"
    fi
}
isolated_path_body()
{
    test -z "${PATHFILE}" && atf_fail "PATHFILE not defined"
    pwd -P >${PATHFILE}
}

isolated_cleanup_head()
{
    atf_set "descr" "Helper test case for the t_isolated test program"
    atf_set "isolated" "yes"
}
isolated_cleanup_body()
{
    test -z "${PATHFILE}" && atf_fail "PATHFILE not defined"
    pwd -P >${PATHFILE}

    mkdir 1
    mkdir 1/1
    mkdir 1/2
    mkdir 1/3
    mkdir 1/3/1
    mkdir 1/3/2
    mkdir 2
    touch 2/1
    touch 2/2
    mkdir 2/3
    touch 2/3/1
}

require_progs_body_head()
{
    atf_set "descr" "Helper test case for the t_require_progs test program"
}
require_progs_body_body()
{
    atf_require_prog ${PROGS}
}

require_progs_head_head()
{
    atf_set "descr" "Helper test case for the t_require_progs test program"
    atf_set "require.progs" "${PROGS}"
}
require_progs_head_body()
{
    :
}

require_user_root_head()
{
    atf_set "descr" "Helper test case for the t_require_user test program"
    atf_set "isolated" "no"
    atf_set "require.user" "root"
}
require_user_root_body()
{
    :
}

require_user_root2_head()
{
    atf_set "descr" "Helper test case for the t_require_user test program"
    atf_set "isolated" "no"
    atf_set "require.user" "root"
}
require_user_root2_body()
{
    :
}

require_user_unprivileged_head()
{
    atf_set "descr" "Helper test case for the t_require_user test program"
    atf_set "isolated" "no"
    atf_set "require.user" "unprivileged"
}
require_user_unprivileged_body()
{
    :
}

require_user_unprivileged2_head()
{
    atf_set "descr" "Helper test case for the t_require_user test program"
    atf_set "isolated" "no"
    atf_set "require.user" "unprivileged"
}
require_user_unprivileged2_body()
{
    :
}

atf_init_test_cases()
{
    atf_add_test_case env_undef
    atf_add_test_case fork_mangle_fds
    atf_add_test_case isolated_path
    atf_add_test_case isolated_cleanup
    # srcdir_exists is not here (while it is in h_cpp.cpp) because of the
    # requirements of the t_srcdir test program (which cannot rely on -s
    # itself to find the source file).
    atf_add_test_case require_progs_body
    atf_add_test_case require_progs_head
    atf_add_test_case require_user_root
    atf_add_test_case require_user_root2
    atf_add_test_case require_user_unprivileged
    atf_add_test_case require_user_unprivileged2
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
