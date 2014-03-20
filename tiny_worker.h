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
#include "tiny_msg.h"
#include <pthread.h>
#include <time.h>

typedef void* (*msg_handler)(void* arg);
typedef enum WORKER_STATUS worker_staus;
typedef struct tiny_worker tworker;


struct tiny_worker{
	pthread_t  id;
	time_t     starttime;
	msg_handler  handler;
	worker_staus status;
	int lock;
	tmsg_queue* queue;
	pthread_mutex_t  mutex;
	pthread_cond_t  cond;
	pthread_attr_t  attr;
};

struct tiny_worker** workers;

struct tiny_worker* tworker_new(msg_handler handler);
int	   tworker_run(tworker* w);
int	   tworker_stop(tworker* w);
int    tworker_transfer_msg(tworker* w, tmsg* buffer);
void   tworker_desrtoy(tworker* w);



#endif /* TINY_WORKER_H_ */
