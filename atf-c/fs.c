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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atf-c/fs.h"
#include "atf-c/text.h"

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

int
atf_fs_cleanup(const char *p)
{
    struct stat sb;

    lstat(p, &sb);

    return atf_fs_cleanup_aux(p, sb.st_dev);
}
