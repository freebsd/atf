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

#include <sys/types.h>

#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

#include "atf-c/sanity.h"
#include "atf-c/signals.h"

/*
 * Define atf_signals_last_signo to the last signal number valid for
 * the system.  This is tricky.  For example, NetBSD defines SIGPWR as
 * the last valid number, whereas Mac OS X defines it as SIGTHR.  Both
 * share the same signal number (32).  If none of these are available,
 * we assume that the highest signal is SIGUSR2.
 *
 * TODO: Make this a configure check that uses kill and finds the first
 * number that returns EINVAL.  The result is probably usable in the
 * shell interface too.
 */
#if defined(SIGTHR) && defined(SIGPWR)
#   if SIGTHR > SIGPWR
#       define LAST_SIGNO SIGTHR
#   elif SIGPWR < SIGTHR
#       define LAST_SIGNO SIGPWR
#   else
#       define LAST_SIGNO SIGPWR
#   endif
#elif defined(SIGTHR)
#   define LAST_SIGNO SIGTHR
#elif defined(SIGPWR)
#   define LAST_SIGNO SIGPWR
#else
#   define LAST_SIGNO SIGUSR2
#endif
const int atf_signals_last_signo = LAST_SIGNO;

/* ---------------------------------------------------------------------
 * The "atf_signal_holder" type.
 * --------------------------------------------------------------------- */

static bool happened[LAST_SIGNO + 1];

void
holder_handler(int signo)
{
    happened[signo] = true;
}

/*
 * Constructors/destructors.
 */

atf_error_t
atf_signal_holder_init(atf_signal_holder_t *sh, int signo)
{
    atf_error_t err;

    err = atf_signal_programmer_init(&sh->m_sp, signo, holder_handler);
    if (atf_is_error(err))
        goto out;

    atf_object_init(&sh->m_object);
    sh->m_signo = signo;
    happened[signo] = false;

    INV(!atf_is_error(err));
out:
    return err;
}

void
atf_signal_holder_fini(atf_signal_holder_t *sh)
{
    atf_signal_programmer_fini(&sh->m_sp);

    if (happened[sh->m_signo])
        kill(getpid(), sh->m_signo);

    atf_object_fini(&sh->m_object);
}

/*
 * Getters.
 */

void
atf_signal_holder_process(atf_signal_holder_t *sh)
{
    if (happened[sh->m_signo]) {
        int ret;
        atf_error_t err;

        atf_signal_programmer_fini(&sh->m_sp);
        happened[sh->m_signo] = false;

        ret = kill(getpid(), sh->m_signo);
        INV(ret != -1);

        err = atf_signal_programmer_init(&sh->m_sp, sh->m_signo,
                                         holder_handler);
        INV(!atf_is_error(err));
    }
}

/* ---------------------------------------------------------------------
 * The "atf_signal_programmer" type.
 * --------------------------------------------------------------------- */

/*
 * Constructors/destructors.
 */

atf_error_t
atf_signal_programmer_init(atf_signal_programmer_t *sp, int signo,
                           atf_signal_handler_t handler)
{
    atf_error_t err;
    struct sigaction sa;

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(signo, &sa, &sp->m_oldsa) == -1)
        err = atf_libc_error(errno, "Failed to program signal %d", signo);
    else {
        atf_object_init(&sp->m_object);
        sp->m_signo = signo;
        err = atf_no_error();
    }

    return err;
}

void
atf_signal_programmer_fini(atf_signal_programmer_t *sp)
{
    if (sigaction(sp->m_signo, &sp->m_oldsa, NULL) == -1)
        UNREACHABLE;

    atf_object_fini(&sp->m_object);
}
