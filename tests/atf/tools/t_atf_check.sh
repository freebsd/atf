h_pass()
{
    echo "Checking command [atf-check $@]"
    atf-check "$@" >tmp || ( cat tmp ; atf_fail "atf-check failed"; )
}

h_fail()
{
    echo "Checking command [atf-check $@]"
    atf-check "$@" 2>tmp \
        && ( cat tmp ; atf_fail "atf-check succeeded but should fail"; )
}


atf_test_case status
status_head()
{
    atf_set "descr" "Checks -s option"
}
status_body()
{
    h_pass -s eq:0 "true"
    h_pass -s ne:0 "false"
    h_pass -s eq:255 "exit 255"
    h_pass -s ne:255 "exit 0"

    h_fail -s eq:256 "exit 256"
    h_fail -s eq:-1 "exit -1"
    h_fail -s ne:256 "true"
    h_fail -s ne:-1 "true"
}

atf_test_case output
output_head()
{
    atf_set "descr" "Checks -s/-e options"
}
output_body()
{
    dd if=/dev/urandom of=tmp1 bs=1k count=1024
    echo "foo bar" >tmp2    

    h_pass -o file:tmp2 "echo foo bar"
    h_pass -e file:tmp2 "echo foo bar >&2"
    h_fail -o file:tmp2 "echo foo ba"
    h_fail -o file:tmp2 "echo foo barr"
    h_fail -o file:tmp2 "echo -n foo bar"
    h_pass -e file:tmp1 "cat tmp1 >&2"
    h_pass -o file:tmp1 "cat tmp1"

    h_pass -o inline:foobar "echo foobar"
    h_pass -e inline:foobar "echo foobar >&2"

    h_pass -o empty -e ignore "echo foo >&2"
    h_pass -o ignore -e ignore  "echo foo >&2"
    h_pass -o ignore -e empty "echo foo"
    h_pass -o ignore -e ignore "echo foo"

    h_pass -o save:tmp3 "echo foo"
    h_pass -e inline:foo "cat tmp3 >&2"
    rm tmp3 || atf_fail "rm failed"
    h_pass -e save:tmp3 "echo foo >&2"
    h_pass -o inline:foo "cat tmp3"
}


atf_init_test_cases()
{
    atf_add_test_case output
    atf_add_test_case status
}
