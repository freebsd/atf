//
// Automated Testing Framework (atf)
//
// Copyright (c) 2008 The NetBSD Foundation, Inc.
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

#if !defined(ATF_C_FS_H)
#define ATF_C_FS_H

#include <stdarg.h>
#include <stdbool.h>

#include <atf-c/dynstr.h>
#include <atf-c/error.h>
#include <atf-c/object.h>

/* ---------------------------------------------------------------------
 * The "atf_fs_path" type.
 * --------------------------------------------------------------------- */

struct atf_fs_path {
    atf_object_t m_object;

    atf_dynstr_t m_data;
};
typedef struct atf_fs_path atf_fs_path_t;

/* Constructors/destructors. */
atf_error_t atf_fs_path_init_ap(atf_fs_path_t *, const char *, va_list);
atf_error_t atf_fs_path_init_fmt(atf_fs_path_t *, const char *, ...);
void atf_fs_path_fini(atf_fs_path_t *);

/* Getters. */
atf_error_t atf_fs_path_branch_path(const atf_fs_path_t *, atf_fs_path_t *);
const char *atf_fs_path_cstring(const atf_fs_path_t *);
atf_error_t atf_fs_path_leaf_name(const atf_fs_path_t *, atf_dynstr_t *);
bool atf_fs_path_is_absolute(const atf_fs_path_t *);
bool atf_fs_path_is_root(const atf_fs_path_t *);

/* Modifiers. */
atf_error_t atf_fs_path_append_ap(atf_fs_path_t *, const char *, va_list);
atf_error_t atf_fs_path_append_fmt(atf_fs_path_t *, const char *, ...);
atf_error_t atf_fs_path_to_absolute(atf_fs_path_t *);

/* Operators. */
bool atf_equal_fs_path_fs_path(const atf_fs_path_t *,
                               const atf_fs_path_t *);

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

atf_error_t atf_fs_cleanup(const atf_fs_path_t *);

#endif // !defined(ATF_C_FS_H)
