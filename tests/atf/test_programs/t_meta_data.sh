#
# Automated Testing Framework (atf)
#
# Copyright (c) 2007, 2008, 2010 The NetBSD Foundation, Inc.
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
    for h in $(get_helpers); do
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile ident_1"
        atf_check -s eq:0 -o ignore -e empty grep passed resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile ident_2"
        atf_check -s eq:0 -o ignore -e empty grep passed resfile
    done
}

# -------------------------------------------------------------------------
# Tests for the "require_arch" meta-data property.
# -------------------------------------------------------------------------

atf_test_case require_arch
require_arch_head()
{
    atf_set "descr" "Tests that the require.arch property works"
}
require_arch_body()
{
    for h in $(get_helpers); do
        echo "Check for the real architecture"
        arch=$(atf-config -t atf_arch)
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v arch='${arch}' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v arch='foo ${arch}' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v arch='${arch} foo' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile

        echo "Some fictitious checks"
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_ARCH=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v arch='foo' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_ARCH=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v arch='foo bar' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_ARCH=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v arch='bar foo' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile

        echo "Now some failures"
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_ARCH=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v arch='bar' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_ARCH=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v arch='bar baz' \
                   require_arch"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
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
    for h in $(get_helpers); do
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile require_config"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
        atf_check -s eq:0 -o ignore -e empty grep "var1 not defined" resfile

        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v var1=foo \
                   require_config"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
        atf_check -s eq:0 -o ignore -e empty grep "var2 not defined" resfile

        atf_check -s eq:0 -o save:stdout -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v var1=foo -v var2=bar \
                   require_config"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e empty grep "var1: foo" stdout
        atf_check -s eq:0 -o ignore -e empty grep "var2: bar" stdout
    done
}

# -------------------------------------------------------------------------
# Tests for the "require_machine" meta-data property.
# -------------------------------------------------------------------------

atf_test_case require_machine
require_machine_head()
{
    atf_set "descr" "Tests that the require.machine property works"
}
require_machine_body()
{
    for h in $(get_helpers); do
        echo "Check for the real machine type"
        machine=$(atf-config -t atf_machine)
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v machine='${machine}' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v machine='foo ${machine}' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v machine='${machine} foo' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile

        echo "Some fictitious checks"
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_MACHINE=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v machine='foo' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_MACHINE=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v machine='foo bar' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_MACHINE=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v machine='bar foo' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile

        echo "Now some failures"
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_MACHINE=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v machine='bar' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "ATF_MACHINE=foo ${h} -s $(atf_get_srcdir) -r resfile \
                   -v machine='bar baz' \
                   require_machine"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
    done
}

# -------------------------------------------------------------------------
# Tests for the "require_progs" meta-data property.
# -------------------------------------------------------------------------

common_tests() {
    where=${1}
    for h in $(get_helpers); do
        # Check absolute paths.
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v 'progs=/bin/cp' \
                   require_progs_${where}"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v \
                   'progs=/bin/__non-existent__' \
                   require_progs_${where}"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile

        # Relative paths are not allowed.
        atf_check -s eq:1 -o ignore -e save:stderr -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v 'progs=bin/cp' \
                   require_progs_${where}"
        atf_check -s eq:0 -o ignore -e empty grep "failed" resfile

        # Check plain file names, searching them in the PATH.
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v 'progs=cp' \
                   require_progs_${where}"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v 'progs=__non-existent__' \
                   require_progs_${where}"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
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

    for h in $(get_helpers); do
        # Check a couple of absolute path names.  The second must make
        # the check fail.
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile \
                   -v 'progs=/bin/cp /bin/__non-existent__' \
                   require_progs_head"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
        atf_check -s eq:0 -o ignore -e empty grep "non-existent" resfile

        # Check a couple of absolute path names.  Both have to be found.
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v 'progs=/bin/cp /bin/ls' \
                   require_progs_head"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile

        # Check an absolute path name and a relative one.  The second must
        # make the check fail.
        atf_check -s ne:0 -o ignore -e save:stderr -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v 'progs=/bin/cp bin/cp' \
                   require_progs_head"
        atf_check -s eq:0 -o ignore -e empty grep "failed" resfile

        # Check an absolute path name and a plain one.  Both have to be
        # found.
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v 'progs=/bin/cp ls' \
                   require_progs_head"
        atf_check -s eq:0 -o ignore -e empty grep "passed" resfile

        # Check an absolute path name and a plain one.  The second must
        # make the check fail.
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile \
                   -v 'progs=/bin/cp __non-existent__' require_progs_head"
        atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
        atf_check -s eq:0 -o ignore -e empty grep "non-existent" resfile
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
    for h in $(get_helpers); do
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v user=root \
                  require_user"
        if [ $(id -u) -eq 0 ]; then
            atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        else
            atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
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
    for h in $(get_helpers); do
        atf_check -s eq:0 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v user=unprivileged \
                   require_user"
        if [ $(id -u) -eq 0 ]; then
            atf_check -s eq:0 -o ignore -e empty grep "skipped" resfile
        else
            atf_check -s eq:0 -o ignore -e empty grep "passed" resfile
        fi
    done
}

atf_test_case require_user_bad
require_user_bad_head()
{
    atf_set "descr" "Tests that passing an invalid value to require.user" \
                    "raises an error"
}
require_user_bad_body()
{
    for h in $(get_helpers); do
        atf_check -s eq:1 -o ignore -e ignore -x \
                  "${h} -s $(atf_get_srcdir) -r resfile -v user=foo \
                  require_user"
        atf_check -s eq:0 -o ignore -e empty grep "reason:.*Invalid" resfile
    done
}

# -------------------------------------------------------------------------
# Main.
# -------------------------------------------------------------------------

atf_init_test_cases()
{
    atf_add_test_case ident
    atf_add_test_case require_arch
    atf_add_test_case require_config
    atf_add_test_case require_machine
    atf_add_test_case require_progs_func
    atf_add_test_case require_progs_header
    atf_add_test_case require_user_root
    atf_add_test_case require_user_unprivileged
    atf_add_test_case require_user_bad
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
