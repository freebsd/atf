/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2010 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/wait.h>

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <atf-c.h>

#include "atf-c/utils.h"

#include "detail/test_helpers.h"

/// Reads the contents of a file into a buffer.
///
/// Up to buflen-1 characters are read into buffer.  If this function returns,
/// the contents read into the buffer are guaranteed to be nul-terminated.
/// Note, however, that if the file contains any nul characters itself,
/// comparing it "as a string" will not work.
///
/// \param path The file to be read, which must exist.
/// \param buffer Buffer into which to store the file contents.
/// \param buflen Size of the target buffer.
///
/// \return The count of bytes read.
static ssize_t
read_file(const char *path, void *const buffer, const size_t buflen)
{
    const int fd = open(path, O_RDONLY);
    ATF_REQUIRE_MSG(fd != -1, "Cannot open %s", path);
    const ssize_t length = read(fd, buffer, buflen - 1);
    close(fd);
    ATF_REQUIRE(length != -1);
    ((char *)buffer)[length] = '\0';
    return length;
}

ATF_TC_WITHOUT_HEAD(cat_file__empty);
ATF_TC_BODY(cat_file__empty, tc)
{
    atf_utils_create_file("file.txt", "");
    atf_utils_redirect(STDOUT_FILENO, "captured.txt");
    atf_utils_cat_file("file.txt", "PREFIX");
    fflush(stdout);
    close(STDOUT_FILENO);

    char buffer[1024];
    read_file("captured.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("", buffer);
}

ATF_TC_WITHOUT_HEAD(cat_file__one_line);
ATF_TC_BODY(cat_file__one_line, tc)
{
    atf_utils_create_file("file.txt", "This is a single line\n");
    atf_utils_redirect(STDOUT_FILENO, "captured.txt");
    atf_utils_cat_file("file.txt", "PREFIX");
    fflush(stdout);
    close(STDOUT_FILENO);

    char buffer[1024];
    read_file("captured.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("PREFIXThis is a single line\n", buffer);
}

ATF_TC_WITHOUT_HEAD(cat_file__several_lines);
ATF_TC_BODY(cat_file__several_lines, tc)
{
    atf_utils_create_file("file.txt", "First\nSecond line\nAnd third\n");
    atf_utils_redirect(STDOUT_FILENO, "captured.txt");
    atf_utils_cat_file("file.txt", ">");
    fflush(stdout);
    close(STDOUT_FILENO);

    char buffer[1024];
    read_file("captured.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ(">First\n>Second line\n>And third\n", buffer);
}

ATF_TC_WITHOUT_HEAD(cat_file__no_newline_eof);
ATF_TC_BODY(cat_file__no_newline_eof, tc)
{
    atf_utils_create_file("file.txt", "Foo\n bar baz");
    atf_utils_redirect(STDOUT_FILENO, "captured.txt");
    atf_utils_cat_file("file.txt", "PREFIX");
    fflush(stdout);
    close(STDOUT_FILENO);

    char buffer[1024];
    read_file("captured.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("PREFIXFoo\nPREFIX bar baz", buffer);
}

ATF_TC_WITHOUT_HEAD(compare_file__empty__match);
ATF_TC_BODY(compare_file__empty__match, tc)
{
    atf_utils_create_file("test.txt", "");
    ATF_REQUIRE(atf_utils_compare_file("test.txt", ""));
}

ATF_TC_WITHOUT_HEAD(compare_file__empty__not_match);
ATF_TC_BODY(compare_file__empty__not_match, tc)
{
    atf_utils_create_file("test.txt", "");
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "\n"));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "foo"));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", " "));
}

ATF_TC_WITHOUT_HEAD(compare_file__short__match);
ATF_TC_BODY(compare_file__short__match, tc)
{
    atf_utils_create_file("test.txt", "this is a short file");
    ATF_REQUIRE(atf_utils_compare_file("test.txt", "this is a short file"));
}

ATF_TC_WITHOUT_HEAD(compare_file__short__not_match);
ATF_TC_BODY(compare_file__short__not_match, tc)
{
    atf_utils_create_file("test.txt", "this is a short file");
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", ""));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "\n"));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "this is a Short file"));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "this is a short fil"));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "this is a short file "));
}

ATF_TC_WITHOUT_HEAD(compare_file__long__match);
ATF_TC_BODY(compare_file__long__match, tc)
{
    char long_contents[3456];
    size_t i = 0;
    for (; i < sizeof(long_contents) - 1; i++)
        long_contents[i] = '0' + (i % 10);
    long_contents[i] = '\0';
    atf_utils_create_file("test.txt", long_contents);

    ATF_REQUIRE(atf_utils_compare_file("test.txt", long_contents));
}

ATF_TC_WITHOUT_HEAD(compare_file__long__not_match);
ATF_TC_BODY(compare_file__long__not_match, tc)
{
    char long_contents[3456];
    size_t i = 0;
    for (; i < sizeof(long_contents) - 1; i++)
        long_contents[i] = '0' + (i % 10);
    long_contents[i] = '\0';
    atf_utils_create_file("test.txt", long_contents);

    ATF_REQUIRE(!atf_utils_compare_file("test.txt", ""));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "\n"));
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", "0123456789"));
    long_contents[i - 1] = 'Z';
    ATF_REQUIRE(!atf_utils_compare_file("test.txt", long_contents));
}

ATF_TC_WITHOUT_HEAD(create_file);
ATF_TC_BODY(create_file, tc)
{
    atf_utils_create_file("test.txt", "This is a test with %d", 12345);

    char buffer[128];
    read_file("test.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("This is a test with 12345", buffer);
}

ATF_TC_WITHOUT_HEAD(fork);
ATF_TC_BODY(fork, tc)
{
    fprintf(stdout, "Should not get into child\n");
    fprintf(stderr, "Should not get into child\n");
    pid_t pid = atf_utils_fork();
    if (pid == 0) {
        fprintf(stdout, "Child stdout\n");
        fprintf(stderr, "Child stderr\n");
        exit(EXIT_SUCCESS);
    }

    int status;
    ATF_REQUIRE(waitpid(pid, &status, 0) != -1);
    ATF_REQUIRE(WIFEXITED(status));
    ATF_REQUIRE_EQ(EXIT_SUCCESS, WEXITSTATUS(status));

    char buffer[1024];
    read_file("atf_utils_fork_out.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("Child stdout\n", buffer);
    read_file("atf_utils_fork_err.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("Child stderr\n", buffer);
}

ATF_TC_WITHOUT_HEAD(free_charpp__empty);
ATF_TC_BODY(free_charpp__empty, tc)
{
    char **array = malloc(sizeof(char *) * 1);
    array[0] = NULL;

    atf_utils_free_charpp(array);
}

ATF_TC_WITHOUT_HEAD(free_charpp__some);
ATF_TC_BODY(free_charpp__some, tc)
{
    char **array = malloc(sizeof(char *) * 4);
    array[0] = strdup("first");
    array[1] = strdup("second");
    array[2] = strdup("third");
    array[3] = NULL;

    atf_utils_free_charpp(array);
}

ATF_TC_WITHOUT_HEAD(redirect__stdout);
ATF_TC_BODY(redirect__stdout, tc)
{
    printf("Buffer this");
    atf_utils_redirect(STDOUT_FILENO, "captured.txt");
    printf("The printed message");
    fflush(stdout);

    char buffer[1024];
    read_file("captured.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("The printed message", buffer);
}

ATF_TC_WITHOUT_HEAD(redirect__stderr);
ATF_TC_BODY(redirect__stderr, tc)
{
    fprintf(stderr, "Buffer this");
    atf_utils_redirect(STDERR_FILENO, "captured.txt");
    fprintf(stderr, "The printed message");
    fflush(stderr);

    char buffer[1024];
    read_file("captured.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ("The printed message", buffer);
}

ATF_TC_WITHOUT_HEAD(redirect__other);
ATF_TC_BODY(redirect__other, tc)
{
    const char *message = "Foo bar\nbaz\n";
    atf_utils_redirect(15, "captured.txt");
    write(15, message, strlen(message));
    close(15);

    char buffer[1024];
    read_file("captured.txt", buffer, sizeof(buffer));
    ATF_REQUIRE_STREQ(message, buffer);
}

static void
fork_and_wait(const int exitstatus, const char* expout, const char* experr)
{
    const pid_t pid = atf_utils_fork();
    if (pid == 0) {
        fprintf(stdout, "Some output\n");
        fprintf(stderr, "Some error\n");
        exit(123);
    }
    atf_utils_wait(pid, exitstatus, expout, experr);
    exit(EXIT_SUCCESS);
}

ATF_TC_WITHOUT_HEAD(wait__ok);
ATF_TC_BODY(wait__ok, tc)
{
    const pid_t control = fork();
    ATF_REQUIRE(control != -1);
    if (control == 0)
        fork_and_wait(123, "Some output\n", "Some error\n");
    else {
        int status;
        ATF_REQUIRE(waitpid(control, &status, 0) != -1);
        ATF_REQUIRE(WIFEXITED(status));
        ATF_REQUIRE_EQ(EXIT_SUCCESS, WEXITSTATUS(status));
    }
}

ATF_TC_WITHOUT_HEAD(wait__invalid_exitstatus);
ATF_TC_BODY(wait__invalid_exitstatus, tc)
{
    const pid_t control = fork();
    ATF_REQUIRE(control != -1);
    if (control == 0)
        fork_and_wait(120, "Some output\n", "Some error\n");
    else {
        int status;
        ATF_REQUIRE(waitpid(control, &status, 0) != -1);
        ATF_REQUIRE(WIFEXITED(status));
        ATF_REQUIRE_EQ(EXIT_FAILURE, WEXITSTATUS(status));
    }
}

ATF_TC_WITHOUT_HEAD(wait__invalid_stdout);
ATF_TC_BODY(wait__invalid_stdout, tc)
{
    const pid_t control = fork();
    ATF_REQUIRE(control != -1);
    if (control == 0)
        fork_and_wait(123, "Some output foo\n", "Some error\n");
    else {
        int status;
        ATF_REQUIRE(waitpid(control, &status, 0) != -1);
        ATF_REQUIRE(WIFEXITED(status));
        ATF_REQUIRE_EQ(EXIT_FAILURE, WEXITSTATUS(status));
    }
}

ATF_TC_WITHOUT_HEAD(wait__invalid_stderr);
ATF_TC_BODY(wait__invalid_stderr, tc)
{
    const pid_t control = fork();
    ATF_REQUIRE(control != -1);
    if (control == 0)
        fork_and_wait(123, "Some output\n", "Some error foo\n");
    else {
        int status;
        ATF_REQUIRE(waitpid(control, &status, 0) != -1);
        ATF_REQUIRE(WIFEXITED(status));
        ATF_REQUIRE_EQ(EXIT_FAILURE, WEXITSTATUS(status));
    }
}

HEADER_TC(include, "atf-c/utils.h");

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, cat_file__empty);
    ATF_TP_ADD_TC(tp, cat_file__one_line);
    ATF_TP_ADD_TC(tp, cat_file__several_lines);
    ATF_TP_ADD_TC(tp, cat_file__no_newline_eof);

    ATF_TP_ADD_TC(tp, compare_file__empty__match);
    ATF_TP_ADD_TC(tp, compare_file__empty__not_match);
    ATF_TP_ADD_TC(tp, compare_file__short__match);
    ATF_TP_ADD_TC(tp, compare_file__short__not_match);
    ATF_TP_ADD_TC(tp, compare_file__long__match);
    ATF_TP_ADD_TC(tp, compare_file__long__not_match);

    ATF_TP_ADD_TC(tp, create_file);

    ATF_TP_ADD_TC(tp, fork);

    ATF_TP_ADD_TC(tp, free_charpp__empty);
    ATF_TP_ADD_TC(tp, free_charpp__some);

    ATF_TP_ADD_TC(tp, redirect__stdout);
    ATF_TP_ADD_TC(tp, redirect__stderr);
    ATF_TP_ADD_TC(tp, redirect__other);

    ATF_TP_ADD_TC(tp, wait__ok);
    ATF_TP_ADD_TC(tp, wait__invalid_exitstatus);
    ATF_TP_ADD_TC(tp, wait__invalid_stdout);
    ATF_TP_ADD_TC(tp, wait__invalid_stderr);

    ATF_TP_ADD_TC(tp, include);

    return atf_no_error();
}
