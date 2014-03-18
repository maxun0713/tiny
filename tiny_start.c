/*
 * tiny_start.c
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#include "tiny_assert.h"
#include "tiny_config.h"
#include "tiny_logger.h"
#include "tiny_server.h"
#include "tiny_module.h"
#include "tiny_worker.h"
#include "tiny_alloc.h"
#include "tiny_signal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>


static
void _daemonlized(){
	pid_t pid, sid;

	pid = fork();
	if (pid < 0) {
	printf("fork process error\n");
	exit(-1);
	} else if (pid > 0) {
	exit(0);
	}

	sid = setsid();
	if (sid < 0) {
	exit(-1);
	}

	signal(SIGHUP, SIG_IGN);

	umask(0);

	pid = fork();
	if (pid<0) {
	printf("fork process error\n");
	} else if(pid > 0){
	printf("\n"
	  "*****************************************************\n"
	  "* Tiny server will be running as a daemon. PID %-8d *\n"
	  "*****************************************************\n\n",
	pid);
	exit(0);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

static
int _save_pid() {
	pid_t pid = getpid();
	char tmp[16];
	FILE *fp = fopen("csp_server.pid", "w");
	T_ERROR_VAL(fp)
	int n = snprintf(tmp, sizeof(tmp), "%d", pid);
	fwrite(tmp, sizeof(char), n, fp);
	fclose(fp);
	return TINY_OK;
}

static
int _set_system_parms(){
	struct rlimit rlim_core;

	/* Soft limit */
	rlim_core.rlim_cur = RLIM_INFINITY;
	/* Hard limit (ceiling for rlim_cur) */
	rlim_core.rlim_max = RLIM_INFINITY;

	return setrlimit(RLIMIT_CORE, &rlim_core);
}

int
start() {
	int i;
	T_ERROR_VAL(_set_system_parms() == TINY_OK)
	T_ERROR_VAL(_save_pid() == TINY_OK)
	if(config.daemonlized) {
		_daemonlized();
	}


	T_ERROR_VAL(tlogger_init(config.logpath, LOG_LEVEL_DEBUG) == TINY_OK)
	T_ERROR_VAL(tserver_init(config.port, config.addr) == TINY_OK)
	T_ERROR_VAL(tsignal_init() == TINY_OK)
	tmodule_init(config.addr);
	struct tiny_module* mod = tmodule_query("gate");

	config.nthread = 1;
	workers = talloc(config.nthread * sizeof(struct tiny_worker*));
	for(i =0; i< config.nthread; i++){
		workers[i] = tworker_new(NULL);
		tworker_run(workers[i]);
	}

	while(1){
		tserver_poll();
	}

	return TINY_OK;
}
