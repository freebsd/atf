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

    cat >Atffile <<EOF
Content-Type: application/X-atf-atffile; version="0"

helper
EOF
}

config_common_head()
{
    atf_set "descr" "Tests that the common.conf configuration file is " \
                    "properly read"
}
config_common_body()
{
    mkdir etc
    cat >etc/common.conf <<EOF
Content-Type: application/X-atf-config; version="0"

foo=first test variable
bar=second test variable
EOF

    create_helper <<EOF
echo "foo: \$(atf_config_get foo)"
echo "bar: \$(atf_config_get bar)"
EOF

    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run helper" 0 stdout ignore
    atf_check "grep 'foo: first test variable' stdout" 0 ignore ignore
    atf_check "grep 'bar: second test variable' stdout" 0 ignore ignore
}

vflag_head()
{
    atf_set "descr" "Tests that the -v flag works and that it properly" \
                    "overrides the values in configuration files"
}
vflag_body()
{
    create_helper <<EOF
if ! atf_config_has testvar; then
    atf_fail "testvar variable not defined"
fi
echo "testvar: \$(atf_config_get testvar)"
EOF

    echo "Checking that 'testvar' is not defined."
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run helper" 1 ignore ignore

    echo "Checking that defining 'testvar' trough '-v' works."
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run -v testvar='a value' helper" \
              0 stdout ignore
    atf_check "grep 'testvar: a value' stdout" 0 ignore ignore

    echo "Checking that defining 'testvar' trough the configuration" \
         "file works."
    mkdir etc
    cat >etc/common.conf <<EOF
Content-Type: application/X-atf-config; version="0"

testvar=value in conf file
EOF
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run helper" 0 stdout ignore
    atf_check "grep 'testvar: value in conf file' stdout" 0 ignore ignore

    echo "Checking that defining 'testvar' trough -v overrides the" \
         "configuration file."
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run -v testvar='a value' helper" \
              0 stdout ignore
    atf_check "grep 'testvar: a value' stdout" 0 ignore ignore
}

atffile_head()
{
    atf_set "descr" "Tests that the variables defined by the Atffile" \
                    "are recognized and that they take the lowest priority"
}
atffile_body()
{
    create_helper <<EOF
if ! atf_config_has testvar; then
    atf_fail "testvar variable not defined"
fi
echo "testvar: \$(atf_config_get testvar)"
EOF

    echo "Checking that 'testvar' is not defined."
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run helper" 1 ignore ignore

    echo "Checking that defining 'testvar' trough the Atffile works."
    echo "testvar=a value" >>Atffile
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run helper" 0 stdout ignore
    atf_check "grep 'testvar: a value' stdout" 0 ignore ignore

    echo "Checking that defining 'testvar' trough the configuration" \
         "file overrides the one in the Atffile."
    mkdir etc
    cat >etc/common.conf <<EOF
Content-Type: application/X-atf-config; version="0"

testvar=value in conf file
EOF
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run helper" 0 stdout ignore
    atf_check "grep 'testvar: value in conf file' stdout" 0 ignore ignore
    rm -rf etc

    echo "Checking that defining 'testvar' trough -v overrides the" \
         "one in the Atffile."
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run -v testvar='new value' helper" \
              0 stdout ignore
    atf_check "grep 'testvar: new value' stdout" 0 ignore ignore
}

atffile_recursive_head()
{
    atf_set "descr" "Tests that variables defined by an Atffile are not" \
                    "inherited by other Atffiles."
}
atffile_recursive_body()
{
    create_helper <<EOF
if ! atf_config_has testvar; then
    atf_fail "testvar variable not defined"
fi
echo "testvar: \$(atf_config_get testvar)"
EOF
    mkdir dir
    mv Atffile helper dir

    echo "Checking that 'testvar' is not inherited."
    cat >Atffile <<EOF
Content-Type: application/X-atf-atffile; version="0"

dir
testvar=a value
EOF
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run" 1 ignore ignore

    echo "Checking that defining 'testvar' in the correct Atffile works."
    echo 'testvar=a value' >>dir/Atffile
    atf_check "ATF_CONFDIR=$(pwd)/etc atf-run" 0 stdout ignore
    atf_check "grep 'testvar: a value' stdout" 0 ignore ignore
}

atf_init_test_cases()
{
    atf_add_test_case config_common
    atf_add_test_case vflag
    atf_add_test_case atffile
    atf_add_test_case atffile_recursive
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
