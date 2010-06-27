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

check_result() {
    file="${1}"; shift

    atf_check -s eq:0 -o match:"${*}" -e empty cat "${file}"
    rm "${file}"
}

atf_test_case expect_pass
expect_pass_head() {
    atf_set "use.fs" "true"
}
expect_pass_body() {
    for h in $(get_helpers); do
        atf_check -s eq:0 "${h}" -r result expect_pass_and_pass
        check_result result "passed"

        atf_check -s eq:1 "${h}" -r result expect_pass_but_fail_requirement
        check_result result "failed: Some reason"

        # atf-sh does not support non-fatal failures yet; skip checks for
        # such conditions.
        case "${h}" in *sh_helpers*) continue ;; esac

        atf_check -s eq:1 -o empty -e match:"Some reason" \
            "${h}" -r result expect_pass_but_fail_check
        check_result result "failed: 1 checks failed"
    done
}

atf_test_case expect_fail
expect_fail_head() {
    atf_set "use.fs" "true"
}
expect_fail_body() {
    for h in $(get_helpers c_helpers cpp_helpers); do
        atf_check -s eq:0 "${h}" -r result expect_fail_and_fail_requirement
        check_result result "expected_failure: Fail reason: The failure"

        atf_check -s eq:1 -e match:"Expected check failure: Fail first: abc" \
            -e not-match:"And fail again" "${h}" -r result expect_fail_but_pass
        check_result result "failed: .*expecting a failure"

        # atf-sh does not support non-fatal failures yet; skip checks for
        # such conditions.
        case "${h}" in *sh_helpers*) continue ;; esac

        atf_check -s eq:0 -e match:"Expected check failure: Fail first: abc" \
            -e match:"Expected check failure: And fail again: def" \
            "${h}" -r result expect_fail_and_fail_check
        check_result result "expected_failure: And fail again: 2 checks" \
            "failed as expected"
    done

    # atf-sh does not support non-fatal failures yet; skip checks for
    # such conditions.
    for h in $(get_helpers sh_helpers); do
        atf_check -s eq:0 "${h}" -r result expect_fail_and_fail_requirement
        check_result result "expected_failure: Fail reason: The failure"

        atf_check -s eq:1 "${h}" -r result expect_fail_but_pass
        check_result result "failed: .*expecting a failure"
    done
}

atf_init_test_cases()
{
    atf_add_test_case expect_pass
    atf_add_test_case expect_fail
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
