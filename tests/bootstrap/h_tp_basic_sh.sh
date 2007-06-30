tc_pass_head()
{
    atf_set "descr" "An empty test case that always passes"
}

tc_pass_body()
{
    atf_pass
}

tc_fail_head()
{
    atf_set "descr" "An empty test case that always fails"
}

tc_fail_body()
{
    atf_fail "On purpose"
}

tc_skip_head()
{
    atf_set "descr" "An empty test case that is always skipped"
}

tc_skip_body()
{
    atf_skip "By design"
}

tc_bogus_head()
{
    atf_set "descr" "A bogus test case that always fails by being" \
                    "incorrectly defined"
}

tc_bogus_body()
{
    :
}

atf_init_test_cases()
{
    atf_add_test_case tc_pass
    atf_add_test_case tc_fail
    atf_add_test_case tc_skip
    atf_add_test_case tc_bogus
}
