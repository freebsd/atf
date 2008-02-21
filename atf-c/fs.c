/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2007, 2008 The NetBSD Foundation, Inc.
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
#include "bconfig.h"
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atf-c/fs.h"
#include "atf-c/sanity.h"
#include "atf-c/text.h"

/* ---------------------------------------------------------------------
 * The "child" error type.
 * --------------------------------------------------------------------- */

/* XXX The need for this conditional here is extremely ugly.  This
 * exception should belong to another module with more process-related
 * utilities. */
#if !defined(HAVE_UNMOUNT)
struct child_error_data {
    char m_cmd[4096];
    int m_state;
};
typedef struct child_error_data child_error_data_t;

static
void
child_format(const atf_error_t err, char *buf, size_t buflen)
{
    const child_error_data_t *data;

    PRE(atf_error_is(err, "child"));

    data = atf_error_data(err);
    snprintf(buf, buflen, "Unknown error while executing \"%s\"; "
             "exit state was %d", data->m_cmd, data->m_state);
}

static
atf_error_t
child_error(const char *cmd, int state)
{
    atf_error_t err;
    child_error_data_t data;

    snprintf(data.m_cmd, sizeof(data.m_cmd), "%s", cmd);
    data.m_state = state;

    err = atf_error_new("child", &data, sizeof(data), child_format);

    return err;
}
#endif /* !defined(HAVE_UNMOUNT) */

/* ---------------------------------------------------------------------
 * The "unknown_file_type" error type.
 * --------------------------------------------------------------------- */

struct unknown_type_error_data {
    const char *m_path;
    int m_type;
};
typedef struct unknown_type_error_data unknown_type_error_data_t;

static
void
unknown_type_format(const atf_error_t err, char *buf, size_t buflen)
{
    const unknown_type_error_data_t *data;

    PRE(atf_error_is(err, "unknown_type"));

    data = atf_error_data(err);
    snprintf(buf, buflen, "Unknown file type %d of %s", data->m_type,
             data->m_path);
}

static
atf_error_t
unknown_type_error(const char *path, int type)
{
    atf_error_t err;
    unknown_type_error_data_t data;

    data.m_path = path;
    data.m_type = type;

    err = atf_error_new("unknown_type", &data, sizeof(data),
                        unknown_type_format);

    return err;
}

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

static atf_error_t cleanup_aux(const atf_fs_path_t *, dev_t, bool);
static atf_error_t cleanup_aux_dir(const char *, dev_t, bool);
static atf_error_t do_unmount(const atf_fs_path_t *);
static atf_error_t normalize(atf_dynstr_t *, char *);
static atf_error_t normalize_ap(atf_dynstr_t *, const char *, va_list);

/* The erase parameter in this routine is to control nested mount points.
 * We want to descend into a mount point to unmount anything that is
 * mounted under it, but we do not want to delete any files while doing
 * this traversal.  In other words, we erase files until we cross the
 * first mount point, and after that point we only scan and unmount. */
static
atf_error_t
cleanup_aux(const atf_fs_path_t *p, dev_t parent_device, bool erase)
{
    const char *pstr = atf_fs_path_cstring(p);
    atf_error_t err;
    atf_fs_stat_t st;

    err = atf_fs_stat_init(&st, p);
    if (atf_is_error(err))
        goto out;

    if (atf_fs_stat_get_type(&st) == atf_fs_stat_dir_type) {
        err = cleanup_aux_dir(pstr, atf_fs_stat_get_device(&st),
                              atf_fs_stat_get_device(&st) == parent_device);
        if (atf_is_error(err))
            goto out_st;
    }

    if (atf_fs_stat_get_device(&st) != parent_device) {
        err = do_unmount(p);
        if (atf_is_error(err))
            goto out_st;
    }

    if (erase) {
        if (atf_fs_stat_get_type(&st) == atf_fs_stat_dir_type) {
            if (rmdir(pstr) == -1)
                err = atf_libc_error(errno, "Cannot remove directory "
                                     "%s", pstr);
            else
                INV(!atf_is_error(err));
        } else {
            if (unlink(pstr) == -1)
                err = atf_libc_error(errno, "Cannot remove file %s", pstr);
            else
                INV(!atf_is_error(err));
        }
    }

out_st:
    atf_fs_stat_fini(&st);
out:
    return err;
}

static
atf_error_t
cleanup_aux_dir(const char *pstr, dev_t this_device, bool erase)
{
    DIR *d;
    atf_error_t err;
    struct dirent *de;

    d = opendir(pstr);
    if (d == NULL) {
        err = atf_libc_error(errno, "Cannot open directory %s", pstr);
        goto out;
    }

    err = atf_no_error();
    while (!atf_is_error(err) && (de = readdir(d)) != NULL) {
        atf_fs_path_t p;

        err = atf_fs_path_init_fmt(&p, "%s/%s", pstr, de->d_name);
        if (!atf_is_error(err)) {
            if (strcmp(de->d_name, ".") != 0 &&
                strcmp(de->d_name, "..") != 0)
                err = cleanup_aux(&p, this_device, erase);

            atf_fs_path_fini(&p);
        }
    }

    closedir(d);

out:
    return err;
}

static
atf_error_t
do_unmount(const atf_fs_path_t *p)
{
    atf_error_t err;
    atf_fs_path_t p2;
    const char *p2str;

    // At least, FreeBSD's unmount(2) requires the path to be absolute.
    // Let's make it absolute in all cases just to be safe that this does
    // not affect other systems.

    err = atf_fs_path_copy(&p2, p);
    if (atf_is_error(err))
        goto out;

    if (!atf_fs_path_is_absolute(&p2)) {
        err = atf_fs_path_to_absolute(&p2);
        if (atf_is_error(err))
            goto out_p2;
    }
    p2str = atf_fs_path_cstring(&p2);

#if defined(HAVE_UNMOUNT)
    if (unmount(p2str, 0) == -1)
        err = atf_libc_error(errno, "Cannot unmount %s", p2str);
    else
        INV(!atf_is_error(err));
#else
    {
        // We could use umount(2) instead if it was available... but
        // trying to do so under, e.g. Linux, is a nightmare because we
        // also have to update /etc/mtab to match what we did.  It is
        // simpler to just leave the system-specific umount(8) tool deal
        // with it, at least for now.
        char *cmd;

        err = atf_text_format(&cmd, "unmount '%s'", p2str);
        if (!atf_is_error(err)) {
            int state = system(cmd);
            if (state == -1)
                err = atf_libc_error(errno, "Failed to run \"%s\"", cmd);
            else if (!WIFEXITED(state) || WEXITSTATUS(state) != EXIT_SUCCESS)
                err = child_error(cmd, state);
            free(cmd);
        }
    }
#endif

out_p2:
    atf_fs_path_fini(&p2);
out:
    return err;
}

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

atf_error_t
atf_fs_path_copy(atf_fs_path_t *dest, const atf_fs_path_t *src)
{
    atf_object_copy(&dest->m_object, &src->m_object);

    return atf_dynstr_copy(&dest->m_data, &src->m_data);
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

    err = atf_dynstr_prepend_fmt(&p->m_data, "%s/", cwd);

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
 * The "atf_fs_path" type.
 * --------------------------------------------------------------------- */

/*
 * Constants.
 */

const int atf_fs_stat_blk_type  = 1;
const int atf_fs_stat_chr_type  = 2;
const int atf_fs_stat_dir_type  = 3;
const int atf_fs_stat_fifo_type = 4;
const int atf_fs_stat_lnk_type  = 5;
const int atf_fs_stat_reg_type  = 6;
const int atf_fs_stat_sock_type = 7;
const int atf_fs_stat_wht_type  = 8;

/*
 * Constructors/destructors.
 */

atf_error_t
atf_fs_stat_init(atf_fs_stat_t *st, const atf_fs_path_t *p)
{
    atf_error_t err;
    const char *pstr = atf_fs_path_cstring(p);

    if (lstat(pstr, &st->m_sb) == -1) {
        err = atf_libc_error(errno, "Cannot get information of %s; "
                             "lstat(2) failed", pstr);
    } else {
        int type = st->m_sb.st_mode & S_IFMT;
        err = atf_no_error();
        switch (type) {
            case S_IFBLK:  st->m_type = atf_fs_stat_blk_type;  break;
            case S_IFCHR:  st->m_type = atf_fs_stat_chr_type;  break;
            case S_IFDIR:  st->m_type = atf_fs_stat_dir_type;  break;
            case S_IFIFO:  st->m_type = atf_fs_stat_fifo_type; break;
            case S_IFLNK:  st->m_type = atf_fs_stat_lnk_type;  break;
            case S_IFREG:  st->m_type = atf_fs_stat_reg_type;  break;
            case S_IFSOCK: st->m_type = atf_fs_stat_sock_type; break;
#if defined(S_IFWHT)
            case S_IFWHT:  st->m_type = atf_fs_stat_wht_type;  break;
#endif
            default:
                err = unknown_type_error(pstr, type);
        }
    }

    if (!atf_is_error(err))
        atf_object_init(&st->m_object);

    return err;
}

void
atf_fs_stat_copy(atf_fs_stat_t *dest, const atf_fs_stat_t *src)
{
    atf_object_copy(&dest->m_object, &src->m_object);

    dest->m_type = src->m_type;
    dest->m_sb = src->m_sb;
}

void
atf_fs_stat_fini(atf_fs_stat_t *st)
{
    atf_object_fini(&st->m_object);
}

/*
 * Getters.
 */

dev_t
atf_fs_stat_get_device(const atf_fs_stat_t *st)
{
    return st->m_sb.st_dev;
}

ino_t
atf_fs_stat_get_inode(const atf_fs_stat_t *st)
{
    return st->m_sb.st_ino;
}

int
atf_fs_stat_get_type(const atf_fs_stat_t *st)
{
    return st->m_type;
}

bool
atf_fs_stat_is_owner_readable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IRUSR;
}

bool
atf_fs_stat_is_owner_writable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IWUSR;
}

bool
atf_fs_stat_is_owner_executable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IXUSR;
}

bool
atf_fs_stat_is_group_readable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IRGRP;
}

bool
atf_fs_stat_is_group_writable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IWGRP;
}

bool
atf_fs_stat_is_group_executable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IXGRP;
}

bool
atf_fs_stat_is_other_readable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IROTH;
}

bool
atf_fs_stat_is_other_writable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IWOTH;
}

bool
atf_fs_stat_is_other_executable(const atf_fs_stat_t *st)
{
    return st->m_sb.st_mode & S_IXOTH;
}

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

atf_error_t
atf_fs_cleanup(const atf_fs_path_t *p)
{
    atf_error_t err;
    atf_fs_stat_t info;

    err = atf_fs_stat_init(&info, p);
    if (atf_is_error(err))
        return err;

    err = cleanup_aux(p, atf_fs_stat_get_device(&info), true);

    atf_fs_stat_fini(&info);

    return err;
}

atf_error_t
atf_fs_mkdtemp(atf_fs_path_t *p)
{
    atf_error_t err;
    char *tmpl;

    tmpl = p->m_data.m_data; // XXX: Ugly
    PRE(strstr(tmpl, "XXXXXX") != NULL);

    if (mkdtemp(tmpl) == NULL)
        err = atf_libc_error(errno, "Cannot create temporary directory "
                             "with template '%s'", tmpl);
    else
        err = atf_no_error();

    return err;
}
