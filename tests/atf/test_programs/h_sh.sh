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

# -------------------------------------------------------------------------
# Helper tests for "t_require_config".
# -------------------------------------------------------------------------

config_unset_head()
{
    atf_set "descr" "Helper test case for the t_config test program"
}
config_unset_body()
{
    atf_config_has 'test' && atf_fail "Test variable already defined"
}

config_empty_head()
{
    atf_set "descr" "Helper test case for the t_config test program"
}
config_empty_body()
{
    atf_check_equal "$(atf_config_get 'test')" ""
}

config_value_head()
{
    atf_set "descr" "Helper test case for the t_config test program"
}
config_value_body()
{
    atf_check_equal "$(atf_config_get 'test')" "foo"
}

config_multi_value_head()
{
    atf_set "descr" "Helper test case for the t_config test program"
}
config_multi_value_body()
{
    atf_check_equal "$(atf_config_get 'test')" "foo bar"
}

# -------------------------------------------------------------------------
# Helper tests for "t_require_env".
# -------------------------------------------------------------------------

env_undef_head()
{
    atf_set "descr" "Helper test case for the t_mangle_fds test program"
}
env_undef_body()
{
    [ -n "${LC_COLLATE}" ] && atf_fail "LC_COLLATE is defined"
    [ -n "${TZ}" ] && atf_fail "TZ is defined"
}

# -------------------------------------------------------------------------
# Helper tests for "t_require_fork".
# -------------------------------------------------------------------------

fork_mangle_fds_head()
{
    atf_set "descr" "Helper test case for the t_mangle_fds test program"
}
fork_mangle_fds_body()
{
    resfd=$(atf_config_get resfd)
    touch in
    exec 0<in
    exec 1>out
    exec 2>err
    eval "exec ${resfd}>res"
}

# -------------------------------------------------------------------------
# Helper tests for "t_require_isolated".
# -------------------------------------------------------------------------

isolated_path_head()
{
    atf_set "descr" "Helper test case for the t_isolated test program"
    atf_set "isolated" "$(atf_config_get isolated)"
}
isolated_path_body()
{
    pwd -P >$(atf_config_get pathfile)
}

isolated_cleanup_head()
{
    atf_set "descr" "Helper test case for the t_isolated test program"
    atf_set "isolated" "yes"
}
isolated_cleanup_body()
{
    pwd -P >$(atf_config_get pathfile)

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

# -------------------------------------------------------------------------
# Helper tests for "t_require_progs".
# -------------------------------------------------------------------------

require_progs_body_head()
{
    atf_set "descr" "Helper test case for the t_require_progs test program"
}
require_progs_body_body()
{
    for p in $(atf_config_get progs); do
        atf_require_prog ${p}
    done
}

require_progs_head_head()
{
    atf_set "descr" "Helper test case for the t_require_progs test program"
    atf_set "require.progs" "$(atf_config_get progs)"
}
require_progs_head_body()
{
    :
}

# -------------------------------------------------------------------------
# Helper tests for "t_require_user".
# -------------------------------------------------------------------------

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

# -------------------------------------------------------------------------
# Main.
# -------------------------------------------------------------------------

atf_init_test_cases()
{
    # Add helper tests for t_config.
    atf_add_test_case config_unset
    atf_add_test_case config_empty
    atf_add_test_case config_value
    atf_add_test_case config_multi_value

    # Add helper tests for t_env.
    atf_add_test_case env_undef

    # Add helper tests for t_fork.
    atf_add_test_case fork_mangle_fds

    # Add helper tests for t_isolated.
    atf_add_test_case isolated_path
    atf_add_test_case isolated_cleanup

    # Add helper tests for t_srcdir.
    # srcdir_exists is not here (while it is in h_cpp.cpp) because of the
    # requirements of the t_srcdir test program (which cannot rely on -s
    # itself to find the source file).

    # Add helper tests for t_require_progs.
    atf_add_test_case require_progs_body
    atf_add_test_case require_progs_head

    # Add helper tests for t_require_user.
    atf_add_test_case require_user_root
    atf_add_test_case require_user_root2
    atf_add_test_case require_user_unprivileged
    atf_add_test_case require_user_unprivileged2
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
