/*
 * tiny_signal.c
 *
 *  Created on: 2014年3月18日
 *      Author: marv
 */


#include "tiny_signal.h"
#include "tiny_logger.h"
#include "tiny_constdef.h"
#include "tiny_assert.h"
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_BACKTRACE_FRAMES 20

void print_stack_trace()
{
	void *b[MAX_BACKTRACE_FRAMES];
	char **info_ptr;
	int i;

	int size = backtrace( b, MAX_BACKTRACE_FRAMES );
	info_ptr = backtrace_symbols( b, size );
	if (info_ptr == NULL)
	{
		tlog(LOG_LEVEL_ERROR , "print stack trace failed : %s", strerror(errno));
		return;
	}

	for (i = 0; i < size; i++ )
	{
		tlog(LOG_LEVEL_ERROR, "%s", info_ptr[i]);
	}

	free( info_ptr );
	exit(1);
}

int
tsignal_init(){
	struct sigaction sa;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = print_stack_trace;

	T_ERROR_VAL(sigaction(SIGSEGV, &sa, NULL) == 0)
	T_ERROR_VAL(sigaction(SIGILL, &sa, NULL) == 0)

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	T_ERROR_VAL(sigaction(SIGPIPE, &sa, NULL) == 0);

	return TINY_OK;
}
