/* 
 * Copyright (C) 2000 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h>
#include "user_util.h"
#include "kern_util.h"
#include "user.h"
#include "signal_user.h"
#include "signal_kern.h"
#include "sysdep/sigcontext.h"
#include "sigcontext.h"

extern int timer_on;

void set_sigstack(void *sig_stack, int size)
{
	stack_t stack;

	stack.ss_sp = (__ptr_t) sig_stack;
	stack.ss_flags = 0;
	stack.ss_size = size - sizeof(void *);
	if(sigaltstack(&stack, NULL) != 0)
		panic("sigaltstack failed");
}

void set_handler(int sig, void (*handler)(int), int flags, ...)
{
	struct sigaction action;
	va_list ap;
	int mask;

	va_start(ap, flags);
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	while((mask = va_arg(ap, int)) != -1){
		sigaddset(&action.sa_mask, mask);
	}
	action.sa_flags = flags;
	action.sa_restorer = NULL;
	if(sigaction(sig, &action, NULL) < 0)
		panic("sigaction failed");
}

int change_sig(int signal, int on)
{
	sigset_t sigset, old;

	sigemptyset(&sigset);
	sigaddset(&sigset, signal);
	sigprocmask(on ? SIG_UNBLOCK : SIG_BLOCK, &sigset, &old);
	return(sigismember(&old, signal));
}

static void change_signals(int type)
{
	sigset_t mask;

	sigemptyset(&mask);
	if(type == SIG_BLOCK) timer_on = 0;
	else {
		timer_on = 1;
		sigaddset(&mask, SIGVTALRM);
		sigaddset(&mask, SIGALRM);
	}
	sigaddset(&mask, SIGIO);
	sigaddset(&mask, SIGWINCH);
	sigaddset(&mask, SIGPROF);
	if(sigprocmask(type, &mask, NULL) < 0)
		panic("Failed to change signal mask - errno = %d", errno);
}

void block_signals(void)
{
	change_signals(SIG_BLOCK);
}

void unblock_signals(void)
{
	change_signals(SIG_UNBLOCK);
}

#define SIGIO_BIT 0
#define SIGVTALRM_BIT 1

static int enable_mask(sigset_t *mask)
{
	int sigs;

	sigs = sigismember(mask, SIGIO) ? 0 : 1 << SIGIO_BIT;
	sigs |= sigismember(mask, SIGVTALRM) ? 0 : 1 << SIGVTALRM_BIT;
	sigs |= sigismember(mask, SIGALRM) ? 0 : 1 << SIGVTALRM_BIT;
	if(timer_on) sigs |= 1 << SIGVTALRM_BIT;
	return(sigs);
}

int set_signals(int enable)
{
	sigset_t mask;
	int ret;

	sigemptyset(&mask);
	if(enable & (1 << SIGIO_BIT)) sigaddset(&mask, SIGIO);
	if(enable & (1 << SIGVTALRM_BIT)){
		timer_on = 1;
		sigaddset(&mask, SIGVTALRM);
		sigaddset(&mask, SIGALRM);
	}
	if(sigprocmask(SIG_UNBLOCK, &mask, &mask) < 0)
		panic("Failed to enable signals");
	ret = enable_mask(&mask);
	sigemptyset(&mask);
	if((enable & (1 << SIGIO_BIT)) == 0) sigaddset(&mask, SIGIO);
	if((enable & (1 << SIGVTALRM_BIT)) == 0){
		timer_on = 0;
	}
	if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
		panic("Failed to block signals");
	return(ret);
}

/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-file-style: "linux"
 * End:
 */