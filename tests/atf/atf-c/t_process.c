/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008, 2009 The NetBSD Foundation, Inc.
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

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <atf-c.h>

#include "atf-c/defs.h"
#include "atf-c/io.h"
#include "atf-c/process.h"
#include "atf-c/sanity.h"

#include "h_lib.h"

atf_error_t atf_process_status_init(atf_process_status_t *, int);

/* ---------------------------------------------------------------------
 * Auxiliary functions for testing of 'atf_process_fork'.
 * --------------------------------------------------------------------- */

/*
 * Testing of atf_process_fork is quite messy.  We want to be able to test
 * all the possible combinations of stdout and stderr behavior to ensure
 * that the streams are manipulated correctly.
 *
 * To do this, the do_fork function is a wrapper for atf_process_fork that
 * issues stream-specific hooks before fork, while the child is running and
 * after the child terminates.  We then provide test cases that just call
 * do_fork with different hooks.
 *
 * The hooks are described by base_stream, and we then have one *_stream
 * type for ever possible stream behavior.
 */

enum out_type { stdout_type, stderr_type };

struct base_stream {
    void (*init)(void *);
    void (*process)(void *, atf_process_child_t *);
    void (*fini)(void *);

    atf_process_stream_t m_sb;
    enum out_type m_type;
};
#define BASE_STREAM(ihook, phook, fhook, type) \
    { .init = ihook, \
      .process = phook, \
      .fini = fhook, \
      .m_type = type }

static
void
check_file(const enum out_type type)
{
    switch (type) {
    case stdout_type:
        ATF_CHECK(grep_file("stdout", "stdout: msg"));
        ATF_CHECK(!grep_file("stdout", "stderr: msg"));
        break;
    case stderr_type:
        ATF_CHECK(grep_file("stderr", "stderr: msg"));
        ATF_CHECK(!grep_file("stderr", "stdout: msg"));
        break;
    default:
        UNREACHABLE;
    }
}

struct capture_stream {
    struct base_stream m_base;

    atf_dynstr_t m_msg;
};
#define CAPTURE_STREAM(type) \
    { .m_base = BASE_STREAM(capture_stream_init, \
                            capture_stream_process, \
                            capture_stream_fini, \
                            type) }

static
void
capture_stream_init(void *v)
{
    struct capture_stream *s = v;

    RE(atf_process_stream_init_capture(&s->m_base.m_sb));
    RE(atf_dynstr_init(&s->m_msg));
}

static
void
capture_stream_process(void *v, atf_process_child_t *c)
{
    struct capture_stream *s = v;

    bool eof;
    switch (s->m_base.m_type) {
    case stdout_type:
        RE(atf_io_readline(atf_process_child_stdout(c),
                           &s->m_msg, &eof));
        break;
    case stderr_type:
        RE(atf_io_readline(atf_process_child_stderr(c),
                           &s->m_msg, &eof));
        break;
    default:
        UNREACHABLE;
    }
}

static
void
capture_stream_fini(void *v)
{
    struct capture_stream *s = v;

    switch (s->m_base.m_type) {
    case stdout_type:
        ATF_CHECK(grep_string(&s->m_msg, "stdout: msg"));
        ATF_CHECK(!grep_string(&s->m_msg, "stderr: msg"));
        break;
    case stderr_type:
        ATF_CHECK(!grep_string(&s->m_msg, "stdout: msg"));
        ATF_CHECK(grep_string(&s->m_msg, "stderr: msg"));
        break;
    default:
        UNREACHABLE;
    }

    atf_dynstr_fini(&s->m_msg);
    atf_process_stream_fini(&s->m_base.m_sb);
}

struct inherit_stream {
    struct base_stream m_base;
    int m_fd;

    int m_old_fd;
};
#define INHERIT_STREAM(type) \
    { .m_base = BASE_STREAM(inherit_stream_init, \
                            NULL, \
                            inherit_stream_fini, \
                            type) }

static
void
inherit_stream_init(void *v)
{
    struct inherit_stream *s = v;
    const char *name;

    RE(atf_process_stream_init_inherit(&s->m_base.m_sb));

    switch (s->m_base.m_type) {
    case stdout_type:
        s->m_fd = STDOUT_FILENO;
        name = "stdout";
        break;
    case stderr_type:
        s->m_fd = STDERR_FILENO;
        name = "stderr";
        break;
    default:
        UNREACHABLE;
        name = NULL;
    }

    s->m_old_fd = dup(s->m_fd);
    ATF_REQUIRE(s->m_old_fd != -1);
    ATF_REQUIRE(close(s->m_fd) != -1);
    ATF_REQUIRE_EQ(open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644),
                   s->m_fd);
}

static
void
inherit_stream_fini(void *v)
{
    struct inherit_stream *s = v;

    ATF_REQUIRE(dup2(s->m_old_fd, s->m_fd) != -1);
    ATF_REQUIRE(close(s->m_old_fd) != -1);

    atf_process_stream_fini(&s->m_base.m_sb);

    check_file(s->m_base.m_type);
}

struct redirect_fd_stream {
    struct base_stream m_base;

    int m_fd;
};
#define REDIRECT_FD_STREAM(type) \
    { .m_base = BASE_STREAM(redirect_fd_stream_init, \
                            NULL, \
                            redirect_fd_stream_fini, \
                            type) }

static
void
redirect_fd_stream_init(void *v)
{
    struct redirect_fd_stream *s = v;

    switch (s->m_base.m_type) {
    case stdout_type:
        s->m_fd = open("stdout", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        break;
    case stderr_type:
        s->m_fd = open("stderr", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        break;
    default:
        UNREACHABLE;
    }
    ATF_REQUIRE(s->m_fd != -1);

    RE(atf_process_stream_init_redirect_fd(&s->m_base.m_sb, s->m_fd));
}

static
void
redirect_fd_stream_fini(void *v)
{
    struct redirect_fd_stream *s = v;

    ATF_REQUIRE(close(s->m_fd) != -1);

    atf_process_stream_fini(&s->m_base.m_sb);

    check_file(s->m_base.m_type);
}

struct redirect_path_stream {
    struct base_stream m_base;

    atf_fs_path_t m_path;
};
#define REDIRECT_PATH_STREAM(type) \
    { .m_base = BASE_STREAM(redirect_path_stream_init, \
                            NULL, \
                            redirect_path_stream_fini, \
                            type) }

static
void
redirect_path_stream_init(void *v)
{
    struct redirect_path_stream *s = v;

    switch (s->m_base.m_type) {
    case stdout_type:
        RE(atf_fs_path_init_fmt(&s->m_path, "stdout"));
        break;
    case stderr_type:
        RE(atf_fs_path_init_fmt(&s->m_path, "stderr"));
        break;
    default:
        UNREACHABLE;
    }

    RE(atf_process_stream_init_redirect_path(&s->m_base.m_sb, &s->m_path));
}

static
void
redirect_path_stream_fini(void *v)
{
    struct redirect_path_stream *s = v;

    atf_process_stream_fini(&s->m_base.m_sb);

    atf_fs_path_fini(&s->m_path);

    check_file(s->m_base.m_type);
}

static
atf_error_t
child_print(const void *v)
{
    const char *msg = v;

    fprintf(stdout, "stdout: %s\n", msg);
    fprintf(stderr, "stderr: %s\n", msg);

    return atf_no_error();
}

static
void
do_fork(const struct base_stream *outfs, void *out,
        const struct base_stream *errfs, void *err)
{
    atf_process_child_t child;
    atf_process_status_t status;

    outfs->init(out);
    errfs->init(err);

    RE(atf_process_fork(&child, child_print,
                        &outfs->m_sb, &errfs->m_sb, "msg"));
    if (outfs->process != NULL)
        outfs->process(out, &child);
    if (errfs->process != NULL)
        errfs->process(err, &child);
    RE(atf_process_child_wait(&child, &status));

    outfs->fini(out);
    errfs->fini(err);

    atf_process_status_fini(&status);
}

/* ---------------------------------------------------------------------
 * Test cases for the "stream" type.
 * --------------------------------------------------------------------- */

ATF_TC(stream_init_capture);
ATF_TC_HEAD(stream_init_capture, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the "
                      "atf_process_stream_init_capture function");
}
ATF_TC_BODY(stream_init_capture, tc)
{
    atf_process_stream_t sb;

    RE(atf_process_stream_init_capture(&sb));

    ATF_CHECK_EQ(atf_process_stream_type(&sb),
                 atf_process_stream_type_capture);

    atf_process_stream_fini(&sb);
}

ATF_TC(stream_init_inherit);
ATF_TC_HEAD(stream_init_inherit, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the "
                      "atf_process_stream_init_inherit function");
}
ATF_TC_BODY(stream_init_inherit, tc)
{
    atf_process_stream_t sb;

    RE(atf_process_stream_init_inherit(&sb));

    ATF_CHECK_EQ(atf_process_stream_type(&sb),
                 atf_process_stream_type_inherit);

    atf_process_stream_fini(&sb);
}

ATF_TC(stream_init_redirect_fd);
ATF_TC_HEAD(stream_init_redirect_fd, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the "
                      "atf_process_stream_init_redirect_fd function");
}
ATF_TC_BODY(stream_init_redirect_fd, tc)
{
    atf_process_stream_t sb;

    RE(atf_process_stream_init_redirect_fd(&sb, 1));

    ATF_CHECK_EQ(atf_process_stream_type(&sb),
                 atf_process_stream_type_redirect_fd);

    atf_process_stream_fini(&sb);
}

ATF_TC(stream_init_redirect_path);
ATF_TC_HEAD(stream_init_redirect_path, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the "
                      "atf_process_stream_init_redirect_path function");
}
ATF_TC_BODY(stream_init_redirect_path, tc)
{
    atf_process_stream_t sb;
    atf_fs_path_t path;

    RE(atf_fs_path_init_fmt(&path, "foo"));
    RE(atf_process_stream_init_redirect_path(&sb, &path));

    ATF_CHECK_EQ(atf_process_stream_type(&sb),
                 atf_process_stream_type_redirect_path);

    atf_process_stream_fini(&sb);
    atf_fs_path_fini(&path);
}

/* ---------------------------------------------------------------------
 * Test cases for the "status" type.
 * --------------------------------------------------------------------- */

static void child_exit_success(void) ATF_DEFS_ATTRIBUTE_NORETURN;
static void child_exit_failure(void) ATF_DEFS_ATTRIBUTE_NORETURN;
static void child_sigkill(void) ATF_DEFS_ATTRIBUTE_NORETURN;
static void child_sigquit(void) ATF_DEFS_ATTRIBUTE_NORETURN;
static void child_sigterm(void) ATF_DEFS_ATTRIBUTE_NORETURN;

void
child_exit_success(void)
{
    exit(EXIT_SUCCESS);
}

void
child_exit_failure(void)
{
    exit(EXIT_FAILURE);
}

void
child_sigkill(void)
{
    kill(getpid(), SIGKILL);
    abort();
}

void
child_sigquit(void)
{
    kill(getpid(), SIGQUIT);
    abort();
}

void
child_sigterm(void)
{
    kill(getpid(), SIGTERM);
    abort();
}

static
int
fork_and_wait_child(void (*child_func)(void))
{
    pid_t pid;
    int status;

    pid = fork();
    ATF_REQUIRE(pid != -1);
    if (pid == 0) {
        status = 0; /* Silence compiler warnings */
        child_func();
        UNREACHABLE;
        abort();
    } else {
        ATF_REQUIRE(waitpid(pid, &status, 0) != 0);
    }

    return status;
}

ATF_TC(status_exited);
ATF_TC_HEAD(status_exited, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the status type for processes "
                      "that exit cleanly");
}
ATF_TC_BODY(status_exited, tc)
{
    {
        const int rawstatus = fork_and_wait_child(child_exit_success);
        atf_process_status_t s;
        RE(atf_process_status_init(&s, rawstatus));
        ATF_CHECK(atf_process_status_exited(&s));
        ATF_CHECK_EQ(atf_process_status_exitstatus(&s), EXIT_SUCCESS);
        ATF_CHECK(!atf_process_status_signaled(&s));
        atf_process_status_fini(&s);
    }

    {
        const int rawstatus = fork_and_wait_child(child_exit_failure);
        atf_process_status_t s;
        RE(atf_process_status_init(&s, rawstatus));
        ATF_CHECK(atf_process_status_exited(&s));
        ATF_CHECK_EQ(atf_process_status_exitstatus(&s), EXIT_FAILURE);
        ATF_CHECK(!atf_process_status_signaled(&s));
        atf_process_status_fini(&s);
    }
}

ATF_TC(status_signaled);
ATF_TC_HEAD(status_signaled, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the status type for processes "
                      "that end due to a signal");
}
ATF_TC_BODY(status_signaled, tc)
{
    {
        const int rawstatus = fork_and_wait_child(child_sigkill);
        atf_process_status_t s;
        RE(atf_process_status_init(&s, rawstatus));
        ATF_CHECK(!atf_process_status_exited(&s));
        ATF_CHECK(atf_process_status_signaled(&s));
        ATF_CHECK_EQ(atf_process_status_termsig(&s), SIGKILL);
        ATF_CHECK(!atf_process_status_coredump(&s));
        atf_process_status_fini(&s);
    }

    {
        const int rawstatus = fork_and_wait_child(child_sigterm);
        atf_process_status_t s;
        RE(atf_process_status_init(&s, rawstatus));
        ATF_CHECK(!atf_process_status_exited(&s));
        ATF_CHECK(atf_process_status_signaled(&s));
        ATF_CHECK_EQ(atf_process_status_termsig(&s), SIGTERM);
        ATF_CHECK(!atf_process_status_coredump(&s));
        atf_process_status_fini(&s);
    }
}

ATF_TC(status_coredump);
ATF_TC_HEAD(status_coredump, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the status type for processes "
                      "that crash");
}
ATF_TC_BODY(status_coredump, tc)
{
    struct rlimit rl;
    rl.rlim_cur = RLIM_INFINITY;
    rl.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CORE, &rl) == -1)
        atf_tc_skip("Cannot unlimit the core file size; check limits "
                    "manually");

    const int rawstatus = fork_and_wait_child(child_sigquit);
    atf_process_status_t s;
    RE(atf_process_status_init(&s, rawstatus));
    ATF_CHECK(!atf_process_status_exited(&s));
    ATF_CHECK(atf_process_status_signaled(&s));
    ATF_CHECK_EQ(atf_process_status_termsig(&s), SIGQUIT);
    ATF_CHECK(atf_process_status_coredump(&s));
    atf_process_status_fini(&s);
}

/* ---------------------------------------------------------------------
 * Tests cases for the free functions.
 * --------------------------------------------------------------------- */

static const int exit_v_null = 1;
static const int exit_v_notnull = 2;

static
atf_error_t
child_cookie(const void *v)
{
    if (v == NULL)
        exit(exit_v_null);
    else
        exit(exit_v_notnull);

    UNREACHABLE;
    return atf_no_error();
}

ATF_TC(fork_cookie);
ATF_TC_HEAD(fork_cookie, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests forking a child, with "
                      "a null and non-null data cookie");
}
ATF_TC_BODY(fork_cookie, tc)
{
    atf_process_stream_t outsb, errsb;

    RE(atf_process_stream_init_inherit(&outsb));
    RE(atf_process_stream_init_inherit(&errsb));

    {
        atf_process_child_t child;
        atf_process_status_t status;

        RE(atf_process_fork(&child, child_cookie, &outsb, &errsb, NULL));
        RE(atf_process_child_wait(&child, &status));

        ATF_CHECK(atf_process_status_exited(&status));
        ATF_CHECK_EQ(atf_process_status_exitstatus(&status), exit_v_null);

        atf_process_status_fini(&status);
    }

    {
        atf_process_child_t child;
        atf_process_status_t status;

        RE(atf_process_fork(&child, child_cookie, &outsb, &errsb, "cookie"));
        RE(atf_process_child_wait(&child, &status));

        ATF_CHECK(atf_process_status_exited(&status));
        ATF_CHECK_EQ(atf_process_status_exitstatus(&status), exit_v_notnull);

        atf_process_status_fini(&status);
    }

    atf_process_stream_fini(&errsb);
    atf_process_stream_fini(&outsb);
}

#define TC_FORK_STREAMS(outlc, outuc, errlc, erruc) \
    ATF_TC(fork_out_ ## outlc ## _err_ ## errlc); \
    ATF_TC_HEAD(fork_out_ ## outlc ## _err_ ## errlc, tc) \
    { \
        atf_tc_set_md_var(tc, "descr", "Tests forking a child, with " \
                          "stdout " #outlc " and stderr " #errlc); \
    } \
    ATF_TC_BODY(fork_out_ ## outlc ## _err_ ## errlc, tc) \
    { \
        struct outlc ## _stream out = outuc ## _STREAM(stdout_type); \
        struct errlc ## _stream err = erruc ## _STREAM(stderr_type); \
        do_fork(&out.m_base, &out, &err.m_base, &err); \
    }

TC_FORK_STREAMS(capture, CAPTURE, capture, CAPTURE);
TC_FORK_STREAMS(capture, CAPTURE, inherit, INHERIT);
TC_FORK_STREAMS(capture, CAPTURE, redirect_fd, REDIRECT_FD);
TC_FORK_STREAMS(capture, CAPTURE, redirect_path, REDIRECT_PATH);
TC_FORK_STREAMS(inherit, INHERIT, capture, CAPTURE);
TC_FORK_STREAMS(inherit, INHERIT, inherit, INHERIT);
TC_FORK_STREAMS(inherit, INHERIT, redirect_fd, REDIRECT_FD);
TC_FORK_STREAMS(inherit, INHERIT, redirect_path, REDIRECT_PATH);
TC_FORK_STREAMS(redirect_fd, REDIRECT_FD, capture, CAPTURE);
TC_FORK_STREAMS(redirect_fd, REDIRECT_FD, inherit, INHERIT);
TC_FORK_STREAMS(redirect_fd, REDIRECT_FD, redirect_fd, REDIRECT_FD);
TC_FORK_STREAMS(redirect_fd, REDIRECT_FD, redirect_path, REDIRECT_PATH);
TC_FORK_STREAMS(redirect_path, REDIRECT_PATH, capture, CAPTURE);
TC_FORK_STREAMS(redirect_path, REDIRECT_PATH, inherit, INHERIT);
TC_FORK_STREAMS(redirect_path, REDIRECT_PATH, redirect_fd, REDIRECT_FD);
TC_FORK_STREAMS(redirect_path, REDIRECT_PATH, redirect_path, REDIRECT_PATH);

#undef TC_FORK_STREAMS

/* ---------------------------------------------------------------------
 * Tests cases for the header file.
 * --------------------------------------------------------------------- */

HEADER_TC(include, "atf-c/process.h", "d_include_process_h.c");

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    /* Add the tests for the "stream" type. */
    ATF_TP_ADD_TC(tp, stream_init_capture);
    ATF_TP_ADD_TC(tp, stream_init_inherit);
    ATF_TP_ADD_TC(tp, stream_init_redirect_fd);
    ATF_TP_ADD_TC(tp, stream_init_redirect_path);

    /* Add the tests for the "status" type. */
    ATF_TP_ADD_TC(tp, status_exited);
    ATF_TP_ADD_TC(tp, status_signaled);
    ATF_TP_ADD_TC(tp, status_coredump);

    /* Add the tests for the free functions. */
    ATF_TP_ADD_TC(tp, fork_cookie);
    ATF_TP_ADD_TC(tp, fork_out_capture_err_capture);
    ATF_TP_ADD_TC(tp, fork_out_capture_err_inherit);
    ATF_TP_ADD_TC(tp, fork_out_capture_err_redirect_fd);
    ATF_TP_ADD_TC(tp, fork_out_capture_err_redirect_path);
    ATF_TP_ADD_TC(tp, fork_out_inherit_err_capture);
    ATF_TP_ADD_TC(tp, fork_out_inherit_err_inherit);
    ATF_TP_ADD_TC(tp, fork_out_inherit_err_redirect_fd);
    ATF_TP_ADD_TC(tp, fork_out_inherit_err_redirect_path);
    ATF_TP_ADD_TC(tp, fork_out_redirect_fd_err_capture);
    ATF_TP_ADD_TC(tp, fork_out_redirect_fd_err_inherit);
    ATF_TP_ADD_TC(tp, fork_out_redirect_fd_err_redirect_fd);
    ATF_TP_ADD_TC(tp, fork_out_redirect_fd_err_redirect_path);
    ATF_TP_ADD_TC(tp, fork_out_redirect_path_err_capture);
    ATF_TP_ADD_TC(tp, fork_out_redirect_path_err_inherit);
    ATF_TP_ADD_TC(tp, fork_out_redirect_path_err_redirect_fd);
    ATF_TP_ADD_TC(tp, fork_out_redirect_path_err_redirect_path);

    /* Add the test cases for the header file. */
    ATF_TP_ADD_TC(tp, include);

    return atf_no_error();
}
