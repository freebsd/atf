/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
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
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atf-c/fs.h"
#include "atf-c/sanity.h"
#include "atf-c/text.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

static
atf_error_t
normalize(atf_dynstr_t *d, char *p)
{
    const char *ptr;
    char *last;
    atf_error_t err;
    bool first;

    PRE(strlen(p) > 0);
    PRE(atf_dynstr_length(d) == 0);

    if (p[0] == '/')
        err = atf_dynstr_append_fmt(d, "/");
    else
        err = atf_no_error();

    first = true;
    ptr = strtok_r(p, "/", &last);
    while (!atf_is_error(err) && ptr != NULL) {
        if (strlen(ptr) > 0) {
            err = atf_dynstr_append_fmt(d, "%s%s", first ? "" : "/", ptr);
            first = false;
        }

        ptr = strtok_r(NULL, "/", &last);
    }

    return err;
}

static
atf_error_t
normalize_ap(atf_dynstr_t *d, const char *p, va_list ap)
{
    char *str;
    atf_error_t err;

    err = atf_dynstr_init(d);
    if (atf_is_error(err))
        goto out;

    err = atf_text_format_ap(&str, p, ap);
    if (atf_is_error(err))
        atf_dynstr_fini(d);
    else {
        err = normalize(d, str);
        free(str);
    }

out:
    return err;
}

static
int
do_unmount(const char *p)
{
#if defined(HAVE_UNMOUNT)
    if (p[0] != '/') {
        char *p2;
        char *curdir;

        curdir = getcwd(NULL, 0);
        atf_text_format(&p2, "%s/%s", curdir, p);
        free(curdir);
        unmount(p2, 0);
        free(p2);
    } else
        unmount(p, 0);
#else
    unimplemented;
#endif
    return 0;
}

static
int
atf_fs_cleanup_aux(const char *p, dev_t parent_device)
{
    struct stat sb;

    lstat(p, &sb);

    if (sb.st_dev != parent_device)
        do_unmount(p);

    if (sb.st_mode & S_IFDIR) {
        DIR *d;
        struct dirent *de;

        d = opendir(p);

        while ((de = readdir(d)) != NULL) {
            char *p2;

            atf_text_format(&p2, "%s/%s", p, de->d_name);

            if (strcmp(de->d_name, ".") != 0 &&
                strcmp(de->d_name, "..") != 0) {
                atf_fs_cleanup_aux(p2, sb.st_dev);
            }

            free(p2);
        }

        closedir(d);

        rmdir(p);
    } else {
        unlink(p);
    }

    return 0;
}

/* ---------------------------------------------------------------------
 * The "atf_fs_path" type.
 * --------------------------------------------------------------------- */

/*
 * Constructors/destructors.
 */

atf_error_t
atf_fs_path_init_ap(atf_fs_path_t *p, const char *fmt, va_list ap)
{
    atf_object_init(&p->m_object);

    return normalize_ap(&p->m_data, fmt, ap);
}

atf_error_t
atf_fs_path_init_fmt(atf_fs_path_t *p, const char *fmt, ...)
{
    va_list ap;
    atf_error_t err;

    va_start(ap, fmt);
    err = atf_fs_path_init_ap(p, fmt, ap);
    va_end(ap);

    return err;
}

void
atf_fs_path_fini(atf_fs_path_t *p)
{
    atf_dynstr_fini(&p->m_data);

    atf_object_fini(&p->m_object);
}

/*
 * Getters.
 */

atf_error_t
atf_fs_path_branch_path(const atf_fs_path_t *p, atf_fs_path_t *bp)
{
    const ssize_t endpos = atf_dynstr_rfind_ch(&p->m_data, '/');
    atf_error_t err;

    if (endpos == atf_dynstr_npos)
        err = atf_fs_path_init_fmt(bp, ".");
    else if (endpos == 0)
        err = atf_fs_path_init_fmt(bp, "/");
    else {
        atf_object_init(&bp->m_object);
        err = atf_dynstr_init_substr(&bp->m_data, &p->m_data, 0, endpos);
    }

#if defined(HAVE_CONST_DIRNAME)
    INV(atf_equal_dynstr_cstring(&bp->m_data,
                                 dirname(atf_dynstr_cstring(&p->m_data))));
#endif /* defined(HAVE_CONST_DIRNAME) */

    return err;
}

const char *
atf_fs_path_cstring(const atf_fs_path_t *p)
{
    return atf_dynstr_cstring(&p->m_data);
}

atf_error_t
atf_fs_path_leaf_name(const atf_fs_path_t *p, atf_dynstr_t *ln)
{
    ssize_t begpos = atf_dynstr_rfind_ch(&p->m_data, '/');
    atf_error_t err;

    if (begpos == atf_dynstr_npos)
        begpos = 0;
    else
        begpos++;

    err = atf_dynstr_init_substr(ln, &p->m_data, begpos, atf_dynstr_npos);

#if defined(HAVE_CONST_BASENAME)
    INV(atf_equal_dynstr_cstring(ln,
                                 basename(atf_dynstr_cstring(&p->m_data))));
#endif /* defined(HAVE_CONST_BASENAME) */

    return err;
}

bool
atf_fs_path_is_absolute(const atf_fs_path_t *p)
{
    return atf_dynstr_cstring(&p->m_data)[0] == '/';
}

bool
atf_fs_path_is_root(const atf_fs_path_t *p)
{
    return atf_equal_dynstr_cstring(&p->m_data, "/");
}

/*
 * Modifiers.
 */

atf_error_t
atf_fs_path_append_ap(atf_fs_path_t *p, const char *fmt, va_list ap)
{
    atf_dynstr_t aux;
    atf_error_t err;

    err = normalize_ap(&aux, fmt, ap);
    if (!atf_is_error(err)) {
        const char *auxstr = atf_dynstr_cstring(&aux);
        const bool needslash = auxstr[0] != '/';

        err = atf_dynstr_append_fmt(&p->m_data, "%s%s",
                                    needslash ? "/" : "", auxstr);

        atf_dynstr_fini(&aux);
    }

    return err;
}

atf_error_t
atf_fs_path_append_fmt(atf_fs_path_t *p, const char *fmt, ...)
{
    va_list ap;
    atf_error_t err;

    va_start(ap, fmt);
    err = atf_fs_path_append_ap(p, fmt, ap);
    va_end(ap);

    return err;
}

atf_error_t
atf_fs_path_to_absolute(atf_fs_path_t *p)
{
    char *cwd;
    atf_error_t err;

    PRE(!atf_fs_path_is_absolute(p));

    cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        err = atf_libc_error(errno, "Cannot determine current directory");
        goto out;
    }

    err = atf_dynstr_prepend_fmt(&p->m_data, "%s", cwd);

    free(cwd);
out:
    return err;
}

/*
 * Operators.
 */

bool atf_equal_fs_path_fs_path(const atf_fs_path_t *p1,
                               const atf_fs_path_t *p2)
{
    return atf_equal_dynstr_dynstr(&p1->m_data, &p2->m_data);
}

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

atf_error_t
atf_fs_cleanup(const atf_fs_path_t *p)
{
    struct stat sb;

    lstat(p->m_data.m_data, &sb);

    atf_fs_cleanup_aux(p->m_data.m_data, sb.st_dev);
    return atf_no_error();
}
