/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008, 2009, 2010 The NetBSD Foundation, Inc.
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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atf-c/config.h"
#include "atf-c/defs.h"
#include "atf-c/env.h"
#include "atf-c/error.h"
#include "atf-c/fs.h"
#include "atf-c/process.h"
#include "atf-c/sanity.h"
#include "atf-c/signals.h"
#include "atf-c/tc.h"
#include "atf-c/tcr.h"
#include "atf-c/text.h"
#include "atf-c/user.h"

/* ---------------------------------------------------------------------
 * Auxiliary types and functions.
 * --------------------------------------------------------------------- */

static atf_error_t check_arch(const char *, void *);
static atf_error_t check_config(const char *, void *);
static atf_error_t check_machine(const char *, void *);
static atf_error_t check_prog(const char *, void *);
static atf_error_t check_prog_in_dir(const char *, void *);
static atf_error_t check_requirements(const atf_tc_t *);
static void fail_internal(const char *, int, const char *, const char *,
                          const char *, va_list,
                          void (*)(atf_dynstr_t *),
                          void (*)(const char *, ...));
static void tc_fail(atf_dynstr_t *) ATF_DEFS_ATTRIBUTE_NORETURN;
static void tc_fail_nonfatal(atf_dynstr_t *);

/* ---------------------------------------------------------------------
 * The "atf_tc" type.
 * --------------------------------------------------------------------- */

/*
 * Constructors/destructors.
 */

atf_error_t
atf_tc_init(atf_tc_t *tc, const char *ident, atf_tc_head_t head,
            atf_tc_body_t body, atf_tc_cleanup_t cleanup,
            const atf_map_t *config)
{
    atf_error_t err;

    atf_object_init(&tc->m_object);

    tc->m_ident = ident;
    tc->m_head = head;
    tc->m_body = body;
    tc->m_cleanup = cleanup;
    tc->m_config = config;

    err = atf_map_init(&tc->m_vars);
    if (atf_is_error(err))
        goto err_object;

    err = atf_tc_set_md_var(tc, "ident", ident);
    if (atf_is_error(err))
        goto err_map;

    err = atf_tc_set_md_var(tc, "timeout", "300");
    if (atf_is_error(err))
        goto err_map;

    /* XXX Should the head be able to return error codes? */
    tc->m_head(tc);

    if (strcmp(atf_tc_get_md_var(tc, "ident"), ident) != 0)
        atf_tc_fail("Test case head modified the read-only 'ident' "
                    "property");

    INV(!atf_is_error(err));
    return err;

err_map:
    atf_map_fini(&tc->m_vars);
err_object:
    atf_object_fini(&tc->m_object);

    return err;
}

atf_error_t
atf_tc_init_pack(atf_tc_t *tc, const atf_tc_pack_t *pack,
                 const atf_map_t *config)
{
    return atf_tc_init(tc, pack->m_ident, pack->m_head, pack->m_body,
                       pack->m_cleanup, config);
}

void
atf_tc_fini(atf_tc_t *tc)
{
    atf_map_fini(&tc->m_vars);

    atf_object_fini(&tc->m_object);
}

/*
 * Getters.
 */

const char *
atf_tc_get_ident(const atf_tc_t *tc)
{
    return tc->m_ident;
}

const char *
atf_tc_get_config_var(const atf_tc_t *tc, const char *name)
{
    const char *val;
    atf_map_citer_t iter;

    PRE(atf_tc_has_config_var(tc, name));
    iter = atf_map_find_c(tc->m_config, name);
    val = atf_map_citer_data(iter);
    INV(val != NULL);

    return val;
}

const char *
atf_tc_get_config_var_wd(const atf_tc_t *tc, const char *name,
                         const char *defval)
{
    const char *val;

    if (!atf_tc_has_config_var(tc, name))
        val = defval;
    else
        val = atf_tc_get_config_var(tc, name);

    return val;
}

const char *
atf_tc_get_md_var(const atf_tc_t *tc, const char *name)
{
    const char *val;
    atf_map_citer_t iter;

    PRE(atf_tc_has_md_var(tc, name));
    iter = atf_map_find_c(&tc->m_vars, name);
    val = atf_map_citer_data(iter);
    INV(val != NULL);

    return val;
}

bool
atf_tc_has_config_var(const atf_tc_t *tc, const char *name)
{
    bool found;
    atf_map_citer_t end, iter;

    if (tc->m_config == NULL)
        found = false;
    else {
        iter = atf_map_find_c(tc->m_config, name);
        end = atf_map_end_c(tc->m_config);
        found = !atf_equal_map_citer_map_citer(iter, end);
    }

    return found;
}

bool
atf_tc_has_md_var(const atf_tc_t *tc, const char *name)
{
    atf_map_citer_t end, iter;

    iter = atf_map_find_c(&tc->m_vars, name);
    end = atf_map_end_c(&tc->m_vars);
    return !atf_equal_map_citer_map_citer(iter, end);
}

/*
 * Modifiers.
 */

atf_error_t
atf_tc_set_md_var(atf_tc_t *tc, const char *name, const char *fmt, ...)
{
    atf_error_t err;
    char *value;
    va_list ap;

    va_start(ap, fmt);
    err = atf_text_format_ap(&value, fmt, ap);
    va_end(ap);

    if (!atf_is_error(err))
        err = atf_map_insert(&tc->m_vars, name, value, true);
    else
        free(value);

    return err;
}

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

static const atf_tc_t *current_tc = NULL;
static const atf_fs_path_t *current_resfile = NULL;
static size_t current_tc_fail_count = 0;

atf_error_t
atf_tc_run(const atf_tc_t *tc, const atf_fs_path_t *resfile)
{
    atf_error_t err;

    atf_reset_exit_checks(); // XXX

    current_tc = tc;
    current_resfile = resfile;
    current_tc_fail_count = 0;

    err = check_requirements(tc);
    if (atf_is_error(err))
        goto out;

    tc->m_body(tc);

    if (current_tc_fail_count == 0)
        atf_tc_pass();
    else
        atf_tc_fail("%d checks failed; see output for more details",
                    current_tc_fail_count);

    current_tc = NULL;
    current_resfile = NULL;
    current_tc_fail_count = 0;

out:
    return err;
}

atf_error_t
atf_tc_cleanup(const atf_tc_t *tc)
{
    if (tc->m_cleanup != NULL)
        tc->m_cleanup(tc);
    return atf_no_error(); // XXX
}

static
atf_error_t
check_arch(const char *arch, void *data)
{
    bool *found = data;

    if (strcmp(arch, atf_config_get("atf_arch")) == 0)
        *found = true;

    return atf_no_error();
}

static
atf_error_t
check_config(const char *var, void *data)
{
    if (!atf_tc_has_config_var(current_tc, var))
        atf_tc_skip("Required configuration variable %s not defined", var);

    return atf_no_error();
}

static
atf_error_t
check_machine(const char *machine, void *data)
{
    bool *found = data;

    if (strcmp(machine, atf_config_get("atf_machine")) == 0)
        *found = true;

    return atf_no_error();
}

struct prog_found_pair {
    const char *prog;
    bool found;
};

static
atf_error_t
check_prog(const char *prog, void *data)
{
    atf_error_t err;
    atf_fs_path_t p;

    err = atf_fs_path_init_fmt(&p, "%s", prog);
    if (atf_is_error(err))
        goto out;

    if (atf_fs_path_is_absolute(&p)) {
        err = atf_fs_eaccess(&p, atf_fs_access_x);
        if (atf_is_error(err)) {
            atf_error_free(err);
            atf_fs_path_fini(&p);
            atf_tc_skip("The required program %s could not be found", prog);
        }
    } else {
        const char *path = atf_env_get("PATH");
        struct prog_found_pair pf;
        atf_fs_path_t bp;

        err = atf_fs_path_branch_path(&p, &bp);
        if (atf_is_error(err))
            goto out_p;

        if (strcmp(atf_fs_path_cstring(&bp), ".") != 0) {
            atf_fs_path_fini(&bp);
            atf_fs_path_fini(&p);
            atf_tc_fail("Relative paths are not allowed when searching for "
                        "a program (%s)", prog);
        }

        pf.prog = prog;
        pf.found = false;
        err = atf_text_for_each_word(path, ":", check_prog_in_dir, &pf);
        if (atf_is_error(err))
            goto out_bp;

        if (!pf.found) {
            atf_fs_path_fini(&bp);
            atf_fs_path_fini(&p);
            atf_tc_skip("The required program %s could not be found in "
                        "the PATH", prog);
        }

out_bp:
        atf_fs_path_fini(&bp);
    }

out_p:
    atf_fs_path_fini(&p);
out:
    return err;
}

static
atf_error_t
check_prog_in_dir(const char *dir, void *data)
{
    struct prog_found_pair *pf = data;
    atf_error_t err;

    if (pf->found)
        err = atf_no_error();
    else {
        atf_fs_path_t p;

        err = atf_fs_path_init_fmt(&p, "%s/%s", dir, pf->prog);
        if (atf_is_error(err))
            goto out_p;

        err = atf_fs_eaccess(&p, atf_fs_access_x);
        if (!atf_is_error(err))
            pf->found = true;
        else {
            atf_error_free(err);
            INV(!pf->found);
            err = atf_no_error();
        }

out_p:
        atf_fs_path_fini(&p);
    }

    return err;
}

static
atf_error_t
check_requirements(const atf_tc_t *tc)
{
    atf_error_t err;

    err = atf_no_error();

    if (atf_tc_has_md_var(tc, "require.arch")) {
        const char *arches = atf_tc_get_md_var(tc, "require.arch");
        bool found = false;

        if (strlen(arches) == 0)
            atf_tc_fail("Invalid value in the require.arch property");
        else {
            err = atf_text_for_each_word(arches, " ", check_arch, &found);
            if (atf_is_error(err))
                goto out;

            if (!found)
                atf_tc_skip("Requires one of the '%s' architectures",
                            arches);
        }
    }

    if (atf_tc_has_md_var(tc, "require.config")) {
        const char *vars = atf_tc_get_md_var(tc, "require.config");

        if (strlen(vars) == 0)
            atf_tc_fail("Invalid value in the require.config property");
        else {
            err = atf_text_for_each_word(vars, " ", check_config, NULL);
            if (atf_is_error(err))
                goto out;
        }
    }

    if (atf_tc_has_md_var(tc, "require.machine")) {
        const char *machines = atf_tc_get_md_var(tc, "require.machine");
        bool found = false;

        if (strlen(machines) == 0)
            atf_tc_fail("Invalid value in the require.machine property");
        else {
            err = atf_text_for_each_word(machines, " ", check_machine,
                                         &found);
            if (atf_is_error(err))
                goto out;

            if (!found)
                atf_tc_skip("Requires one of the '%s' machine types",
                            machines);
        }
    }

    if (atf_tc_has_md_var(tc, "require.progs")) {
        const char *progs = atf_tc_get_md_var(tc, "require.progs");

        if (strlen(progs) == 0)
            atf_tc_fail("Invalid value in the require.progs property");
        else {
            err = atf_text_for_each_word(progs, " ", check_prog, NULL);
            if (atf_is_error(err))
                goto out;
        }
    }

    if (atf_tc_has_md_var(tc, "require.user")) {
        const char *u = atf_tc_get_md_var(tc, "require.user");

        if (strcmp(u, "root") == 0) {
            if (!atf_user_is_root())
                atf_tc_skip("Requires root privileges");
        } else if (strcmp(u, "unprivileged") == 0) {
            if (atf_user_is_root())
                atf_tc_skip("Requires an unprivileged user");
        } else
            atf_tc_fail("Invalid value in the require.user property");
    }

    INV(!atf_is_error(err));
out:
    return err;
}

static
void
tc_fail(atf_dynstr_t *msg)
{
    atf_tcr_t tcr;
    atf_error_t err;

    PRE(current_tc != NULL);

    err = atf_tcr_init_reason_fmt(&tcr, atf_tcr_failed_state, "%s",
                                  atf_dynstr_cstring(msg));
    if (atf_is_error(err))
        abort();

    atf_tcr_write(&tcr, current_resfile); // XXX: Handle errors.

    atf_tcr_fini(&tcr);
    atf_dynstr_fini(msg);

    exit(EXIT_FAILURE);
}

static
void
tc_fail_nonfatal(atf_dynstr_t *msg)
{
    fprintf(stderr, "%s\n", atf_dynstr_cstring(msg));
    atf_dynstr_fini(msg);

    current_tc_fail_count++;
}

void
atf_tc_fail(const char *fmt, ...)
{
    va_list ap;
    atf_tcr_t tcr;
    atf_error_t err;

    PRE(current_tc != NULL);

    va_start(ap, fmt);
    err = atf_tcr_init_reason_ap(&tcr, atf_tcr_failed_state, fmt, ap);
    va_end(ap);
    if (atf_is_error(err))
        abort();

    atf_tcr_write(&tcr, current_resfile); // XXX: Handle errors.

    atf_tcr_fini(&tcr);

    exit(EXIT_FAILURE);
}

void
atf_tc_fail_nonfatal(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");

    current_tc_fail_count++;
}

void
atf_tc_fail_check(const char *file, int line, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fail_internal(file, line, "Check failed", "*** ", fmt, ap,
                  tc_fail_nonfatal, atf_tc_fail_nonfatal);
    va_end(ap);
}

void
atf_tc_fail_requirement(const char *file, int line, const char *fmt, ...)
{
    va_list ap;

    atf_reset_exit_checks();

    va_start(ap, fmt);
    fail_internal(file, line, "Requirement failed", "", fmt, ap,
                  tc_fail, atf_tc_fail);
    va_end(ap);

    UNREACHABLE;
    abort();
}

static
void
fail_internal(const char *file, int line, const char *reason,
              const char *prefix, const char *fmt, va_list ap,
              void (*failfunc)(atf_dynstr_t *),
              void (*backupfunc)(const char *, ...))
{
    va_list ap2;
    atf_error_t err;
    atf_dynstr_t msg;

    err = atf_dynstr_init_fmt(&msg, "%s%s:%d: %s: ", prefix, file, line,
                              reason);
    if (atf_is_error(err))
        goto backup;

    va_copy(ap2, ap);
    err = atf_dynstr_append_ap(&msg, fmt, ap2);
    va_end(ap2);
    if (atf_is_error(err)) {
        atf_dynstr_fini(&msg);
        goto backup;
    }

    va_copy(ap2, ap);
    failfunc(&msg);
    return;

backup:
    atf_error_free(err);
    va_copy(ap2, ap);
    backupfunc(fmt, ap2);
    va_end(ap2);
}

void
atf_tc_pass(void)
{
    atf_tcr_t tcr;
    atf_error_t err;

    PRE(current_tc != NULL);

    err = atf_tcr_init(&tcr, atf_tcr_passed_state);
    if (atf_is_error(err))
        abort();

    atf_tcr_write(&tcr, current_resfile); // XXX: Handle errors.

    atf_tcr_fini(&tcr);

    exit(EXIT_SUCCESS);
}

void
atf_tc_require_prog(const char *prog)
{
    atf_error_t err;

    err = check_prog(prog, NULL);
    if (atf_is_error(err)) {
        atf_error_free(err);
        atf_tc_fail("atf_tc_require_prog failed"); /* XXX Correct? */
    }
}

void
atf_tc_skip(const char *fmt, ...)
{
    va_list ap;
    atf_tcr_t tcr;
    atf_error_t err;

    PRE(current_tc != NULL);

    va_start(ap, fmt);
    err = atf_tcr_init_reason_ap(&tcr, atf_tcr_skipped_state, fmt, ap);
    va_end(ap);
    if (atf_is_error(err))
        abort();

    atf_tcr_write(&tcr, current_resfile); // XXX: Handle errors.

    atf_tcr_fini(&tcr);

    exit(EXIT_SUCCESS);
}
