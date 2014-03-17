/*
 * tiny_worker.h
 *
 *  Created on: 2014年3月6日
 *      Author: marv
 */

#ifndef TINY_WORKER_H_
#define TINY_WORKER_H_

#include "tiny_constdef.h"
#include "tiny_types.h"
#include <pthread.h>
#include <time.h>

typedef void* (*msg_handler)(void* arg);
typedef enum WORKER_STATUS worker_staus;

struct tiny_worker_parm{
	pthread_mutex_t  mutex;
	pthread_cond_t  cond;
};


struct tiny_worker{
	pthread_t  id;
	time_t     starttime;
	msg_handler  handler;
	worker_staus status;
	int lock;
	struct write_buffer* head;
	struct write_buffer* tail;
	struct tiny_worker_parm* wp;
};

struct tiny_worker* tworker_new(msg_handler handler, struct tiny_worker_parm* wp);
int	   tworker_run(struct tiny_worker* w);
int	   tworker_stop(struct tiny_worker* w);

#endif /* TINY_WORKER_H_ */
