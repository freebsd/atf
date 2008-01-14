#
# Automated Testing Framework (atf)
#
# Copyright (c) 2008 The NetBSD Foundation, Inc.
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

kill_tree()
{
    $(atf-config -t atf_libexecdir)/atf-kill-tree "${@}"
}

helper()
{
    signals="${*}"
    cat >helper.sh <<EOF
#! /bin/sh

for s in ${signals}; do
    trap 'echo "Got \${s}" >helper.out; exit 0;' \${s}
done
sleep 600 &
touch helper.out
wait \$!
exit 1
EOF
    chmod +x helper.sh

    ./helper.sh &
    echo "Helper is process $!" 1>&2
    pid=$!

    # Must wait until the child process has set up the signal handlers.
    while test ! -f helper.out; do sleep 1; done
}

tree_helper()
{
    cat >tree_helper.sh <<EOF
#! /bin/sh

level=\${1}

trap 'echo "Got SIGTERM" >pids/\$$; exit 0;' SIGTERM
sleep 600 &
touch pids/\$$

if test \${level} -gt 0; then
    ( ./tree_helper.sh \$((\${level} - 1))) &
    ( ./tree_helper.sh \$((\${level} - 1))) &
fi

wait \$!
exit 1
EOF
    chmod +x tree_helper.sh
    mkdir pids

    ./tree_helper.sh 2 &
    echo "Parent helper process is $!" 1>&2
    parent_pid=$!

    # Must wait until all the children processes have set up their signal
    # handlers.
    cd pids
    set -- dummy *
    while test $# -ne 8; do
        sleep 1
        set -- dummy *
    done
    cd -
    shift
    echo "All children: ${*}"
    all_pids="${*}"
}

atf_test_case no_pid
no_pid_head()
{
    atf_set "descr" "Checks that an error is raised if no PID is provided"
}
no_pid_body()
{
    atf_check 'kill_tree' 1 null stderr
    atf_check 'grep "ERROR.*process identifier" stderr' 0 ignore null
}

atf_test_case single_process
single_process_head()
{
    atf_set "descr" "Checks that killing a single process works"
}
single_process_body()
{
    helper SIGTERM
    atf_check "kill_tree ${pid}" 0 ignore null
    wait ${pid}
    atf_check "grep 'Got SIGTERM' helper.out" 0 ignore null
}

atf_test_case s_flag
s_flag_head()
{
    atf_set "descr" "Checks that the signal to be sent can be changed" \
                    "through the -s flag"
}
s_flag_body()
{
    helper SIGTERM SIGHUP
    atf_check "kill_tree -s 1 ${pid}" 0 ignore null
    wait ${pid}
    atf_check "grep 'Got SIGTERM' helper.out" 1 null null
    atf_check "grep 'Got SIGHUP' helper.out" 0 ignore null
}

atf_test_case simple_tree
simple_tree_head()
{
    atf_set "descr" "Checks that killing a process kills all of its" \
                    "children"
}
simple_tree_body()
{
    # Note: This check is extremely simple.  All the real meat (which is
    # fairly complex) lives in the unit-tests for the procs module.

    tree_helper
    atf_check "kill_tree ${parent_pid}" 0 ignore null
    wait ${parent_pid}

    for p in ${all_pids}; do
        atf_check "grep 'Got SIGTERM' pids/${p}" 0 ignore null
    done
}

atf_init_test_cases()
{
    atf_add_test_case no_pid
    atf_add_test_case single_process
    atf_add_test_case s_flag
    atf_add_test_case simple_tree
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
