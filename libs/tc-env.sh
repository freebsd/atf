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

#
# Based on an input file describing the environment variables that have
# to be sanitized when running a test case, generates the necessary C++
# or POSIX shell code to do the appropriate changes on these variables.
# Having a single source for the possible variables ensures that the two
# interfaces are kept consistent.
#

Prog_Name=${0##*/}

# -------------------------------------------------------------------------
# C++ output.
# -------------------------------------------------------------------------

process_cpp()
{
    name=${1} defined=${2} default=${3}

    if [ ${defined} = yes ]; then
        case ${default} in
        __WORKDIR__)
            echo "atf::env::set(\"${name}\", workdir);"
            ;;
        *)
            echo "atf::env::set(\"${name}\", \"${default}\");"
            ;;
        esac
    else
        echo "atf::env::unset(\"${name}\");"
    fi
}

# -------------------------------------------------------------------------
# POSIX shell output.
# -------------------------------------------------------------------------

process_sh()
{
    name=${1} defined=${2} default=${3}

    if [ ${defined} = yes ]; then
        case ${default} in
        __WORKDIR__)
            echo "${name}=\$(pwd)"
            ;;
        *)
            echo "${name}=\"${default}\")"
            ;;
        esac
        echo "export ${name}"
    else
        echo "unset ${name}"
    fi
}

# -------------------------------------------------------------------------
# Miscellaneous functions and main program.
# -------------------------------------------------------------------------

err()
{
    echo "${Prog_Name}:" "${@}" 1>&2
    exit 1
}

main()
{
    lang=${1}

    IFS='
'
    set -- $(cat)
    shift # Skip header

    for line in "${@}"; do
        IFS=!
        process_${lang} ${line}
    done

    exit 0
}

main "${@}"

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
