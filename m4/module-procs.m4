dnl
dnl Automated Testing Framework (atf)
dnl
dnl Copyright (c) 2008 The NetBSD Foundation, Inc.
dnl All rights reserved.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl 3. All advertising materials mentioning features or use of this
dnl    software must display the following acknowledgement:
dnl        This product includes software developed by the NetBSD
dnl        Foundation, Inc. and its contributors.
dnl 4. Neither the name of The NetBSD Foundation nor the names of its
dnl    contributors may be used to endorse or promote products derived
dnl    from this software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
dnl CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
dnl INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
dnl MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
dnl IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
dnl DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
dnl DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
dnl GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
dnl INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
dnl IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
dnl OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
dnl IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl

AC_DEFUN([ATF_MODULE_PROCS], [
    AC_MSG_CHECKING(whether signals sent to a stopped process work)
    cat >./conftest.sh <<EOF
rm -f conftest.sig conftest.ready
aux() {
    trap "touch conftest.sig; exit 0" TERM
    touch conftest.ready
    while :; do sleep 1; done
}
aux &
pid=\$!
while ! test -f conftest.ready; do sleep 1; done
kill -s STOP \${pid} || exit 1
kill -s TERM \${pid} || exit 1
tries=30
ret=1
while test \${ret} -eq 1 -a \${tries} -gt 0; do
    sleep 1
    test -f conftest.sig && ret=0
    tries=\$((\${tries} - 1))
done
kill -s KILL \${pid}
exit \${ret}
EOF
    ${SHELL} ./conftest.sh >/dev/null 2>&1
    if test $? -eq 0; then
        AC_MSG_RESULT(yes)
        AC_DEFINE([SUPPORT_SIGNAL_WHILE_STOPPED], [1],
                  [Define to 1 if signals sent to a stopped process work])
    else
        AC_MSG_RESULT(no)
    fi
    rm -f ./conftest.sh ./conftest.sig ./conftest.ready

    AC_CHECK_LIB(kvm, kvm_getprocs, have_libkvm=yes, have_libkvm=no)
    AC_CHECK_LIB(c, kvm_getprocs, have_kvm_wo_lib=yes, have_kvm_wo_lib=no)
    AC_MSG_CHECKING(whether we have /dev/mem)
    if test -b /dev/mem -o -c /dev/mem; then
        have_dev_mem=yes
    else
        have_dev_mem=no
    fi
    AC_MSG_RESULT(${have_dev_mem})

    AC_MSG_CHECKING(whether KVM is usable)
    if test \( ${have_libkvm} = yes -o ${have_kvm_wo_lib} = yes \) -a \
            ${have_dev_mem} = yes
    then
        AC_DEFINE([HAVE_KVM_GETPROCS], [1],
                  [Define to 1 if you have KVM and it is usable])
        if ${have_kvm_wo_lib} = yes; then
            AC_MSG_RESULT(yes, and is in libc)
        else
            AC_MSG_RESULT(yes, and is in libkvm)
            AC_SUBST(KVM_LIBS, -lkvm)
        fi
    else
        AC_MSG_RESULT(no)
    fi

    AC_MSG_CHECKING(for the kern.proc sysctl subtree)
    msg=$(sysctl kern.proc 2>&1)
    if echo ${msg} | grep 'kern\.proc' >/dev/null 2>&1 &&
       echo ${msg} | grep 'ps' >/dev/null 2>&1
    then
        AC_MSG_RESULT(yes)
        AC_DEFINE([HAVE_SYSCTL_KERN_PROC], [1],
                  [Define to 1 if you have the kern.proc sysctl tree])
    else
        AC_MSG_RESULT(no)
    fi

    AC_MSG_CHECKING(for the Linux proc file system)
    if test $(uname) = Linux -a -d /proc; then
        AC_MSG_RESULT(yes)
        AC_DEFINE([HAVE_LINUX_PROCFS], [1],
                  [Define to 1 if you have the Linux proc file system])
    else
        AC_MSG_RESULT(no)
    fi
])
