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
# Tests for the "ident" meta-data property.
# -------------------------------------------------------------------------

atf_test_case ident
ident_head()
{
    atf_set "descr" "Tests that the ident property works"
}
ident_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 ident_1 3>resout" 0 ignore ignore
        atf_check "grep passed resout" 0 ignore null
        atf_check "${h} -s ${srcdir} -r3 ident_2 3>resout" 0 ignore ignore
        atf_check "grep passed resout" 0 ignore null
    done
}

# -------------------------------------------------------------------------
# Tests for the "isolated" meta-data property.
# -------------------------------------------------------------------------

atf_test_case isolated_default
isolated_default_head()
{
    atf_set "descr" "Tests that the 'isolated' test property works" \
                    "when the user does not override the work directory"
}
isolated_default_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(cd $(atf-config -t atf_workdir) && pwd -P)

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -v isolated=no \
                   -v pathfile=$(pwd)/expout isolated_path" 0 ignore ignore
        atf_check "pwd -P" 0 expout null

        atf_check "${h} -s ${srcdir} -v isolated=yes \
                  -v pathfile=$(pwd)/path isolated_path" 0 ignore ignore
        atf_check "grep '^${tmpdir}/atf.' <path" 0 ignore ignore
    done
}

atf_test_case isolated_tmpdir
isolated_tmpdir_head()
{
    atf_set "descr" "Tests that the 'isolated' test property works" \
                    "when the user overrides the work directory through" \
                    "the ATF_WORKDIR environment variable"
}
isolated_tmpdir_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(pwd -P)/workdir
    mkdir ${tmpdir}

    for h in ${h_cpp} ${h_sh}; do
        atf_check "ATF_WORKDIR=${tmpdir} ${h} -s ${srcdir} -v isolated=no \
                   -v pathfile=$(pwd)/expout isolated_path" 0 ignore ignore
        atf_check "pwd -P" 0 expout null

        atf_check "ATF_WORKDIR=${tmpdir} ${h} -s ${srcdir} -v isolated=yes \
                   -v pathfile=$(pwd)/path isolated_path" 0 ignore ignore
        atf_check "grep '^${tmpdir}/atf.' <path" 0 ignore ignore
    done
}

atf_test_case isolated_conf
isolated_conf_head()
{
    atf_set "descr" "Tests that the 'isolated' test property works" \
                    "when the user overrides the work directory through" \
                    "the test program's 'workdir' configuration option"
}
isolated_conf_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(pwd -P)/workdir
    mkdir ${tmpdir}

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -v isolated=no \
                   -v pathfile=$(pwd)/expout -v workdir=${tmpdir} \
                   isolated_path" 0 ignore ignore
        atf_check "pwd -P" 0 expout null

        atf_check "${h} -s ${srcdir} -v isolated=yes \
                   -v pathfile=$(pwd)/path -v workdir=${tmpdir} \
                   isolated_path" 0 ignore ignore
        atf_check "grep '^${tmpdir}/atf.' <path" 0 ignore null
    done
}

check_emptydir()
{
    dir=${1} what=${2}
    set -- ${dir}/atf.*
    [ -f ${1} ] && \
        atf_fail "${what} did not properly remove all temporary files"
}

atf_test_case isolated_cleanup
isolated_cleanup_head()
{
    atf_set "descr" "Tests that the directories used to isolate test" \
                    "cases are properly cleaned up once the test case" \
                    "exits"
}
isolated_cleanup_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh
    tmpdir=$(pwd -P)/workdir
    mkdir ${tmpdir}

    for h in ${h_cpp} ${h_sh}; do
        # First try to clean a work directory that, supposedly, does not
        # have any subdirectories.  Do this for both isolated set to yes
        # and no, as there used to be a bug that make the latter case
        # leave a temporary file behind.
        atf_check "${h} -s ${srcdir} -v isolated=no \
                   -v pathfile=$(pwd)/path -v workdir=${tmpdir} \
                   isolated_path" 0 ignore ignore
        check_emptydir ${tmpdir} isolated=no
        atf_check "${h} -s ${srcdir} -v isolated=yes \
                   -v pathfile=$(pwd)/path -v workdir=${tmpdir} \
                   isolated_path" 0 ignore ignore
        atf_check "test -d $(cat path)" 1 null null
        check_emptydir ${tmpdir} isolated=yes

        # Now do the same but with a work directory that has subdirectories.
        # The program will have to recurse into them to clean them all.
        atf_check "${h} -s ${srcdir} -v pathfile=$(pwd)/path \
                   -v workdir=${tmpdir} isolated_cleanup" 0 ignore ignore
        atf_check "test -d $(cat path)" 1 null null
    done
}

# -------------------------------------------------------------------------
# Tests for the "require_config" meta-data property.
# -------------------------------------------------------------------------

atf_test_case require_config
require_config_head()
{
    atf_set "descr" "Tests that the require.config property works"
}
require_config_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 require_config 3>resout" \
                  0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null
        atf_check 'grep "var1 not defined" resout' 0 ignore null

        atf_check "${h} -s ${srcdir} -r3 -v var1=foo \
                   require_config 3>resout" 0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null
        atf_check 'grep "var2 not defined" resout' 0 ignore null

        atf_check "${h} -s ${srcdir} -r3 -v var1=foo -v var2=bar \
                   require_config 3>resout" 0 stdout ignore
        atf_check 'grep "passed" resout' 0 ignore null
        atf_check 'grep "var1: foo" stdout' 0 ignore null
        atf_check 'grep "var2: bar" stdout' 0 ignore null
    done
}

# -------------------------------------------------------------------------
# Tests for the "require_progs" meta-data property.
# -------------------------------------------------------------------------

common_tests() {
    where=${1}
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        # Check absolute paths.
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=/bin/cp' \
                   require_progs_${where} 3>resout" 0 ignore ignore
        atf_check 'grep "passed" resout' 0 ignore null
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=/bin/__non-existent__' \
                   require_progs_${where} 3>resout" 0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null

        # Relative paths are not allowed.
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=bin/cp' \
                   require_progs_${where} 3>resout" 1 ignore stderr
        atf_check 'grep "failed" resout' 0 ignore null

        # Check plain file names, searching them in the PATH.
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=cp' \
                   require_progs_${where} 3>resout" 0 ignore ignore
        atf_check 'grep "passed" resout' 0 ignore null
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=__non-existent__' \
                   require_progs_${where} 3>resout" 0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null
    done
}

# XXX This does not belong here.  This is a unit test.
atf_test_case require_progs_func
require_progs_func_head()
{
    atf_set "descr" "Tests that atf_require_prog works"
}
require_progs_func_body()
{
    common_tests body
}

atf_test_case require_progs_header
require_progs_header_head()
{
    atf_set "descr" "Tests that require.progs works"
}
require_progs_header_body()
{
    common_tests head

    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        # Check a couple of absolute path names.  The second must make
        # the check fail.
        atf_check "${h} -s ${srcdir} -r3 \
                   -v 'progs=/bin/cp /bin/__non-existent__' \
                   require_progs_head 3>resout" 0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null
        atf_check 'grep "non-existent" resout' 0 ignore null

        # Check a couple of absolute path names.  Both have to be found.
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=/bin/cp /bin/ls' \
                   require_progs_head 3>resout" 0 ignore ignore
        atf_check 'grep "passed" resout' 0 ignore null

        # Check an absolute path name and a relative one.  The second must
        # make the check fail.
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=/bin/cp bin/cp' \
                   require_progs_head 3>resout" 1 ignore stderr
        atf_check 'grep "failed" resout' 0 ignore null

        # Check an absolute path name and a plain one.  Both have to be
        # found.
        atf_check "${h} -s ${srcdir} -r3 -v 'progs=/bin/cp ls' \
                   require_progs_head 3>resout" 0 ignore ignore
        atf_check 'grep "passed" resout' 0 ignore null

        # Check an absolute path name and a plain one.  The second must
        # make the check fail.
        atf_check "${h} -s ${srcdir} -r3 \
                   -v 'progs=/bin/cp __non-existent__' require_progs_head \
                   3>resout" 0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null
        atf_check 'grep "non-existent" resout' 0 ignore null
    done
}

# -------------------------------------------------------------------------
# Tests for the "require_user" meta-data property.
# -------------------------------------------------------------------------

count_lines()
{
    wc -l $1 | awk '{ print $1 }'
}

atf_test_case require_user_root
require_user_root_head()
{
    atf_set "descr" "Tests that 'require.user=root' works"
}
require_user_root_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 require_user_root \
            3>resout" 0 ignore ignore
        if [ $(id -u) -eq 0 ]; then
            atf_check 'grep "passed" resout' 0 ignore null
        else
            atf_check 'grep "skipped" resout' 0 ignore null
        fi
    done
}

atf_test_case require_user_unprivileged
require_user_unprivileged_head()
{
    atf_set "descr" "Tests that 'require.user=unprivileged' works"
}
require_user_unprivileged_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 require_user_unprivileged \
            3>resout" 0 ignore ignore
        if [ $(id -u) -eq 0 ]; then
            atf_check 'grep "skipped" resout' 0 ignore null
        else
            atf_check 'grep "passed" resout' 0 ignore null
        fi
    done
}

atf_test_case require_user_multiple
require_user_multiple_head()
{
    atf_set "descr" "Tests that multiple skip results raised by the" \
                    "handling of require.user do not abort the program" \
                    "prematurely"
}
require_user_multiple_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 'require_user*' 3>resout" \
            0 ignore ignore
        grep "skipped" resout >skips
        [ $(count_lines skips) -lt 2 ] && \
            atf_fail "Test program aborted prematurely"
        [ $(count_lines skips) -gt 2 ] && \
            atf_fail "Test program returned more skips than expected"
    done
}

# -------------------------------------------------------------------------
# Main.
# -------------------------------------------------------------------------

atf_init_test_cases()
{
    atf_add_test_case ident
    atf_add_test_case isolated_default
    atf_add_test_case isolated_tmpdir
    atf_add_test_case isolated_conf
    atf_add_test_case isolated_cleanup
    atf_add_test_case require_config
    atf_add_test_case require_progs_func
    atf_add_test_case require_progs_header
    atf_add_test_case require_user_root
    atf_add_test_case require_user_unprivileged
    atf_add_test_case require_user_multiple
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
