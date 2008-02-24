//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007, 2008 The NetBSD Foundation, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this
//    software must display the following acknowledgement:
//        This product includes software developed by the NetBSD
//        Foundation, Inc. and its contributors.
// 4. Neither the name of The NetBSD Foundation nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
// CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atf-c.h>

#include "atf-c/env.h"
#include "atf-c/fs.h"
#include "atf-c/text.h"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

#define CE(stm) ATF_CHECK(!atf_is_error(stm))

static
void
write_cwd(const atf_tc_t *tc, const char *confvar)
{
    char *cwd;
    const char *p;
    FILE *f;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), confvar, &p));

    f = fopen(p, "w");
    if (f == NULL)
        atf_tc_fail("Could not open %s for writing", p);

    cwd = getcwd(NULL, 0);
    if (cwd == NULL)
        atf_tc_fail("Could not get current directory");

    fprintf(f, "%s\n", cwd);

    fclose(f);
}

static
void
safe_mkdir(const char* path)
{
    if (mkdir(path, 0755) == -1)
        atf_tc_fail("mkdir(2) of %s failed", path);
}

static
void
safe_remove(const char* path)
{
    if (unlink(path) == -1)
        atf_tc_fail("unlink(2) of %s failed", path);
}

static
void
touch(const char *path)
{
    int fd;
    fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd == -1)
        atf_tc_fail("Could not create file %s", path);
    close(fd);
}

// ------------------------------------------------------------------------
// Helper tests for "t_cleanup".
// ------------------------------------------------------------------------

ATF_TC_WITH_CLEANUP(cleanup_pass);
ATF_TC_HEAD(cleanup_pass, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_cleanup test "
               "program");
}
ATF_TC_BODY(cleanup_pass, tc)
{
    const char *tmp;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));
    touch(tmp);
}
ATF_TC_CLEANUP(cleanup_pass, tc)
{
    const char *tmp;
    bool cleanup;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));
    CE(atf_map_get_bool(atf_tc_get_config(tc), "cleanup", &cleanup));

    if (cleanup)
        safe_remove(tmp);
}

ATF_TC_WITH_CLEANUP(cleanup_fail);
ATF_TC_HEAD(cleanup_fail, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_cleanup test "
                   "program");
}
ATF_TC_BODY(cleanup_fail, tc)
{
    const char *tmp;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));
    touch(tmp);
    atf_tc_fail("On purpose");
}
ATF_TC_CLEANUP(cleanup_fail, tc)
{
    const char *tmp;
    bool cleanup;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));
    CE(atf_map_get_bool(atf_tc_get_config(tc), "cleanup", &cleanup));

    if (cleanup)
        safe_remove(tmp);
}

ATF_TC_WITH_CLEANUP(cleanup_skip);
ATF_TC_HEAD(cleanup_skip, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_cleanup test "
                   "program");
}
ATF_TC_BODY(cleanup_skip, tc)
{
    const char *tmp;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));
    touch(tmp);
    atf_tc_skip("On purpose");
}
ATF_TC_CLEANUP(cleanup_skip, tc)
{
    const char *tmp;
    bool cleanup;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));
    CE(atf_map_get_bool(atf_tc_get_config(tc), "cleanup", &cleanup));

    if (cleanup)
        safe_remove(tmp);
}

ATF_TC_WITH_CLEANUP(cleanup_curdir);
ATF_TC_HEAD(cleanup_curdir, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_cleanup test "
                   "program");
}
ATF_TC_BODY(cleanup_curdir, tc)
{
    FILE *f;

    f = fopen("oldvalue", "w");
    if (f == NULL)
        atf_tc_fail("Failed to create oldvalue file");
    fprintf(f, "1234");
    fclose(f);
}
ATF_TC_CLEANUP(cleanup_curdir, tc)
{
    FILE *f;

    f = fopen("oldvalue", "r");
    if (f != NULL) {
        int i;
        fscanf(f, "%d", &i);
        printf("Old value: %d", i);
        fclose(f);
    }
}

ATF_TC_WITH_CLEANUP(cleanup_sigterm);
ATF_TC_HEAD(cleanup_sigterm, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_cleanup test "
                   "program");
}
ATF_TC_BODY(cleanup_sigterm, tc)
{
    char *nofile;
    const char *tmp;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));

    touch(tmp);
    kill(getpid(), SIGTERM);

    CE(atf_text_format(&nofile, "%s.no", tmp));
    touch(nofile);
    free(nofile);
}
ATF_TC_CLEANUP(cleanup_sigterm, tc)
{
    const char *tmp;

    CE(atf_map_get_cstring(atf_tc_get_config(tc), "tmpfile", &tmp));

    safe_remove(tmp);
}

ATF_TC_WITH_CLEANUP(cleanup_fork);
ATF_TC_HEAD(cleanup_fork, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_cleanup test "
                   "program");
}
ATF_TC_BODY(cleanup_fork, tc)
{
}
ATF_TC_CLEANUP(cleanup_fork, tc)
{
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    close(3);
}

// ------------------------------------------------------------------------
// Helper tests for "t_config".
// ------------------------------------------------------------------------

ATF_TC(config_unset);
ATF_TC_HEAD(config_unset, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_config test "
                   "program");
}
ATF_TC_BODY(config_unset, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    atf_map_citer_t iter;

    iter = atf_map_find_c(config, "test");
    ATF_CHECK(atf_equal_map_citer_map_citer(iter, atf_map_end_c(config)));
}

ATF_TC(config_empty);
ATF_TC_HEAD(config_empty, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_config test "
                   "program");
}
ATF_TC_BODY(config_empty, tc)
{
    atf_map_citer_t iter;
    const char *value;

    iter = atf_map_find_c(atf_tc_get_config(tc), "test");
    value = atf_map_citer_data(iter);

    ATF_CHECK(strlen(value) == 0);
}

ATF_TC(config_value);
ATF_TC_HEAD(config_value, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_config test "
                   "program");
}
ATF_TC_BODY(config_value, tc)
{
    atf_map_citer_t iter;
    const char *value;

    iter = atf_map_find_c(atf_tc_get_config(tc), "test");
    value = atf_map_citer_data(iter);

    ATF_CHECK(strcmp(value, "foo") == 0);
}

ATF_TC(config_multi_value);
ATF_TC_HEAD(config_multi_value, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_config test "
                   "program");
}
ATF_TC_BODY(config_multi_value, tc)
{
    atf_map_citer_t iter;
    const char *value;

    iter = atf_map_find_c(atf_tc_get_config(tc), "test");
    value = atf_map_citer_data(iter);

    ATF_CHECK(strcmp(value, "foo bar") == 0);
}

// ------------------------------------------------------------------------
// Helper tests for "t_env".
// ------------------------------------------------------------------------

ATF_TC(env_home);
ATF_TC_HEAD(env_home, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_env test "
                   "program");
}
ATF_TC_BODY(env_home, tc)
{
    atf_fs_path_t cwd, home;
    atf_fs_stat_t stcwd, sthome;

    ATF_CHECK(atf_env_has("HOME"));

    CE(atf_fs_getcwd(&cwd));
    CE(atf_fs_path_init_fmt(&home, "%s", atf_env_get("HOME")));

    CE(atf_fs_stat_init(&stcwd, &cwd));
    CE(atf_fs_stat_init(&sthome, &home));

    ATF_CHECK_EQUAL(atf_fs_stat_get_device(&stcwd),
                    atf_fs_stat_get_device(&sthome));
    ATF_CHECK_EQUAL(atf_fs_stat_get_inode(&stcwd),
                    atf_fs_stat_get_inode(&sthome));
}

ATF_TC(env_list);
ATF_TC_HEAD(env_list, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_env test "
                   "program");
}
ATF_TC_BODY(env_list, tc)
{
    system("env");
}

// ------------------------------------------------------------------------
// Helper tests for "t_fork".
// ------------------------------------------------------------------------

ATF_TC(fork_mangle_fds);
ATF_TC_HEAD(fork_mangle_fds, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_fork test "
                   "program");
}
ATF_TC_BODY(fork_mangle_fds, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    long resfd;

    CE(atf_map_get_long(config, "resfd", &resfd));

    if (close(STDIN_FILENO) == -1)
        atf_tc_fail("Failed to close stdin");
    if (close(STDOUT_FILENO) == -1)
        atf_tc_fail("Failed to close stdout");
    if (close(STDERR_FILENO) == -1)
        atf_tc_fail("Failed to close stderr");
    if (close(resfd) == -1)
        atf_tc_fail("Failed to close results descriptor");

#if defined(F_CLOSEM)
    if (fcntl(0, F_CLOSEM) == -1)
        atf_tc_fail("Failed to close everything");
#endif
}

ATF_TC(fork_stop);
ATF_TC_HEAD(fork_stop, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_fork test "
                   "program");
}
ATF_TC_BODY(fork_stop, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    FILE *f;
    const char *dfstr, *pfstr;

    CE(atf_map_get_cstring(config, "donefile", &dfstr));
    CE(atf_map_get_cstring(config, "pidfile", &pfstr));

    f = fopen(pfstr, "w");
    if (f == NULL)
        atf_tc_fail("Failed to create pidfile %s", pfstr);
    fprintf(f, "%d", getpid());
    fclose(f);
    printf("Wrote pid file\n");

    printf("Waiting for done file\n");
    while (access(dfstr, F_OK) != 0)
        usleep(10000);
    printf("Exiting\n");
}

ATF_TC(fork_umask);
ATF_TC_HEAD(fork_umask, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_fork test "
                   "program");
}
ATF_TC_BODY(fork_umask, tc)
{
    printf("umask: %04o\n", umask(0));
}

// ------------------------------------------------------------------------
// Helper tests for "t_meta_data".
// ------------------------------------------------------------------------

ATF_TC(ident_1);
ATF_TC_HEAD(ident_1, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
}
ATF_TC_BODY(ident_1, tc)
{
    ATF_CHECK(strcmp(atf_tc_get_var(tc, "ident"), "ident_1") == 0);
}

ATF_TC(ident_2);
ATF_TC_HEAD(ident_2, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
}
ATF_TC_BODY(ident_2, tc)
{
    ATF_CHECK(strcmp(atf_tc_get_var(tc, "ident"), "ident_2") == 0);
}

ATF_TC(require_arch);
ATF_TC_HEAD(require_arch, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *arch;

    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");

    atf_map_get_cstring_wd(config, "arch", "not-set", &arch);
    atf_tc_set_var(tc, "require.arch", "%s", arch);
}
ATF_TC_BODY(require_arch, tc)
{
}

ATF_TC(require_config);
ATF_TC_HEAD(require_config, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
    atf_tc_set_var(tc, "require.config", "var1 var2");
}
ATF_TC_BODY(require_config, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *v1, *v2;

    CE(atf_map_get_cstring(config, "var1", &v1));
    CE(atf_map_get_cstring(config, "var2", &v2));

    printf("var1: %s\n", v1);
    printf("var2: %s\n", v2);
}

ATF_TC(require_machine);
ATF_TC_HEAD(require_machine, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *machine;

    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");

    atf_map_get_cstring_wd(config, "machine", "not-set", &machine);
    atf_tc_set_var(tc, "require.machine", "%s", machine);
}
ATF_TC_BODY(require_machine, tc)
{
}

ATF_TC(require_progs_body);
ATF_TC_HEAD(require_progs_body, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
}
ATF_TC_BODY(require_progs_body, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *progs;

    CE(atf_map_get_cstring(config, "progs", &progs));
    /* TODO: Add call to require progs. */
}

ATF_TC(require_progs_head);
ATF_TC_HEAD(require_progs_head, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *progs;

    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
    atf_map_get_cstring_wd(config, "progs", "not-set", &progs);
    atf_tc_set_var(tc, "require.progs", "%s", progs);
}
ATF_TC_BODY(require_progs_head, tc)
{
}

ATF_TC(require_user_root);
ATF_TC_HEAD(require_user_root, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
    atf_tc_set_var(tc, "require.user", "root");
}
ATF_TC_BODY(require_user_root, tc)
{
}

ATF_TC(require_user_root2);
ATF_TC_HEAD(require_user_root2, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
    atf_tc_set_var(tc, "require.user", "root");
}
ATF_TC_BODY(require_user_root2, tc)
{
}

ATF_TC(require_user_unprivileged);
ATF_TC_HEAD(require_user_unprivileged, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
    atf_tc_set_var(tc, "require.user", "unprivileged");
}
ATF_TC_BODY(require_user_unprivileged, tc)
{
}

ATF_TC(require_user_unprivileged2);
ATF_TC_HEAD(require_user_unprivileged2, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
    atf_tc_set_var(tc, "require.user", "unprivileged");
}
ATF_TC_BODY(require_user_unprivileged2, tc)
{
}

ATF_TC(timeout);
ATF_TC_HEAD(timeout, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *timeout;

    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");
    atf_map_get_cstring_wd(config, "timeout", "0", &timeout);
    atf_tc_set_var(tc, "timeout", "%s", timeout);
}
ATF_TC_BODY(timeout, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    long s;

    CE(atf_map_get_long(config, "sleep", &s));
    sleep(s);
}

ATF_TC(timeout2);
ATF_TC_HEAD(timeout2, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *timeout2;

    atf_tc_set_var(tc, "descr", "Helper test case for the t_meta_data "
                   "test program");

    atf_map_get_cstring_wd(config, "timeout2", "0", &timeout2);
    atf_tc_set_var(tc, "timeout", "%s", timeout2);
}
ATF_TC_BODY(timeout2, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    long s;

    CE(atf_map_get_long(config, "sleep2", &s));
    sleep(s);
}

// ------------------------------------------------------------------------
// Helper tests for "t_srcdir".
// ------------------------------------------------------------------------

ATF_TC(srcdir_exists);
ATF_TC_HEAD(srcdir_exists, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_srcdir test "
                   "program");
}
ATF_TC_BODY(srcdir_exists, tc)
{
    const atf_map_t *config = atf_tc_get_config(tc);
    const char *srcdir;
    atf_fs_path_t p;
    bool b;

    CE(atf_map_get_cstring(config, "srcdir", &srcdir));

    CE(atf_fs_path_init_fmt(&p, "%s/datafile", srcdir));
    CE(atf_fs_exists(&p, &b));
    if (!b)
        atf_tc_fail("Cannot find datafile");
    atf_fs_path_fini(&p);
}

// ------------------------------------------------------------------------
// Helper tests for "t_status".
// ------------------------------------------------------------------------

ATF_TC(status_newlines_fail);
ATF_TC_HEAD(status_newlines_fail, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_status test "
                   "program");
}
ATF_TC_BODY(status_newlines_fail, tc)
{
    atf_tc_fail("First line\nSecond line");
}

ATF_TC(status_newlines_skip);
ATF_TC_HEAD(status_newlines_skip, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_status test "
                   "program");
}
ATF_TC_BODY(status_newlines_skip, tc)
{
    atf_tc_skip("First line\nSecond line");
}

// ------------------------------------------------------------------------
// Helper tests for "t_workdir".
// ------------------------------------------------------------------------

ATF_TC(workdir_path);
ATF_TC_HEAD(workdir_path, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_workdir test "
                   "program");
}
ATF_TC_BODY(workdir_path, tc)
{
    write_cwd(tc, "pathfile");
}

ATF_TC(workdir_cleanup);
ATF_TC_HEAD(workdir_cleanup, tc)
{
    atf_tc_set_var(tc, "descr", "Helper test case for the t_workdir test "
                   "program");
}
ATF_TC_BODY(workdir_cleanup, tc)
{
    write_cwd(tc, "pathfile");

    safe_mkdir("1");
    safe_mkdir("1/1");
    safe_mkdir("1/2");
    safe_mkdir("1/3");
    safe_mkdir("1/3/1");
    safe_mkdir("1/3/2");
    safe_mkdir("2");
    touch("2/1");
    touch("2/2");
    safe_mkdir("2/3");
    touch("2/3/1");
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_TP_ADD_TCS(tp)
{
    // Add helper tests for t_cleanup.
    ATF_TP_ADD_TC(tp, cleanup_pass);
    ATF_TP_ADD_TC(tp, cleanup_fail);
    ATF_TP_ADD_TC(tp, cleanup_skip);
    ATF_TP_ADD_TC(tp, cleanup_curdir);
    ATF_TP_ADD_TC(tp, cleanup_sigterm);
    ATF_TP_ADD_TC(tp, cleanup_fork);

    // Add helper tests for t_config.
    ATF_TP_ADD_TC(tp, config_unset);
    ATF_TP_ADD_TC(tp, config_empty);
    ATF_TP_ADD_TC(tp, config_value);
    ATF_TP_ADD_TC(tp, config_multi_value);

    // Add helper tests for t_env.
    ATF_TP_ADD_TC(tp, env_home);
    ATF_TP_ADD_TC(tp, env_list);

    // Add helper tests for t_fork.
    ATF_TP_ADD_TC(tp, fork_mangle_fds);
    ATF_TP_ADD_TC(tp, fork_stop);
    ATF_TP_ADD_TC(tp, fork_umask);

    // Add helper tests for t_meta_data.
    ATF_TP_ADD_TC(tp, ident_1);
    ATF_TP_ADD_TC(tp, ident_2);
    ATF_TP_ADD_TC(tp, require_arch);
    ATF_TP_ADD_TC(tp, require_config);
    ATF_TP_ADD_TC(tp, require_machine);
    ATF_TP_ADD_TC(tp, require_progs_body);
    ATF_TP_ADD_TC(tp, require_progs_head);
    ATF_TP_ADD_TC(tp, require_user_root);
    ATF_TP_ADD_TC(tp, require_user_root2);
    ATF_TP_ADD_TC(tp, require_user_unprivileged);
    ATF_TP_ADD_TC(tp, require_user_unprivileged2);
    ATF_TP_ADD_TC(tp, timeout);
    ATF_TP_ADD_TC(tp, timeout2);

    // Add helper tests for t_srcdir.
    ATF_TP_ADD_TC(tp, srcdir_exists);

    // Add helper tests for t_status.
    ATF_TP_ADD_TC(tp, status_newlines_fail);
    ATF_TP_ADD_TC(tp, status_newlines_skip);

    // Add helper tests for t_workdir.
    ATF_TP_ADD_TC(tp, workdir_path);
    ATF_TP_ADD_TC(tp, workdir_cleanup);

    return atf_no_error();
}
