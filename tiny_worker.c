/*
 * tiny_work.c
 *
 *  Created on: 2014年3月6日
 *      Author: marv
 */


#include "tiny_worker.h"
#include "tiny_alloc.h"
#include "tiny_assert.h"
#include <stdio.h>


void* _test(void* arg){
	tmsg* msg = (tmsg*)arg;
	printf("%s\n", (char*)msg->data);
	return NULL;
}

tworker*
tworker_new(msg_handler handler){
	//T_ERROR_PTR(handler)

	struct tiny_worker* w = talloc(sizeof(*w));
	T_ERROR_PTR(w)
	w->handler = _test;
	w->id = -1;
	w->starttime = -1;
	w->status = WORKER_STATUS_IDLE;
	w->lock = 0;
	w->queue = tmsg_queue_new();
	pthread_mutex_init(&w->mutex, NULL);
	pthread_cond_init(&w->cond, NULL);
	pthread_attr_init(&w->attr);
	pthread_attr_init(&w->attr);
	pthread_attr_setscope(&w->attr, PTHREAD_SCOPE_SYSTEM );
	pthread_attr_setdetachstate(&w->attr, PTHREAD_CREATE_JOINABLE );
	return w;
}


void
cond_block(tworker* w){
	pthread_mutex_lock(&w->mutex);
	if(tmsg_queue_empty(w->queue) || w->status == WORKER_STATUS_STOPPED){
		if(w->status == WORKER_STATUS_STOPPED){
			pthread_mutex_unlock(&w->mutex);
			pthread_exit(NULL);
		}

		pthread_cond_wait(&w->cond, &w->mutex);
	}

	pthread_mutex_unlock(&w->mutex);
}


void*
_work(void* arg){
	struct tiny_worker* w= (struct tiny_worker*)arg;
	T_ERROR_PTR(w);
	while(__sync_lock_test_and_set(&w->lock,1)) {}   //insure lock is modified
	__sync_lock_release(&w->lock);

	for(;;){
		cond_block(w);

		pthread_mutex_lock(&w->mutex);
		tmsg msg;
		int ret = tmsg_queue_pop(w->queue, &msg);
		if(ret == TINY_OK && w->handler)
		{
			w->handler(&msg);
		}
		pthread_mutex_unlock(&w->mutex);
	}

	return NULL;
}


int
tworker_run(tworker* w){
	T_ERROR_VAL(w)
	T_ERROR_VAL(w->status != WORKER_STATUS_RUNNING)

	while(__sync_lock_test_and_set(&w->lock,1)) {}
	int ret = pthread_create(&w->id, &w->attr, _work, w);
	if(ret == 0){
		w->starttime = time(NULL);
		w->status = WORKER_STATUS_RUNNING;
	}
	__sync_lock_release(&w->lock);

	return ret;
}


int
tworker_stop(tworker* w){
	while(__sync_lock_test_and_set(&w->lock,1)) {}
	w->status = WORKER_STATUS_STOPPED;
	__sync_lock_release(&w->lock);

	pthread_mutex_lock(&w->mutex);
	pthread_cond_signal(&w->cond);
	pthread_mutex_unlock(&w->mutex);

	void** status = NULL;
	pthread_join(w->id, status);
	return 0;
}


int
tworker_transfer_msg(tworker* w, tmsg* msg){
	T_ERROR_VAL(w)
	T_ERROR_VAL(msg)

	pthread_mutex_lock(&w->mutex);
	int ret = tmsg_queue_push(w->queue, msg);

	pthread_cond_signal(&w->cond);
	pthread_mutex_unlock(&w->mutex);

	return ret;
}


void
tworker_desrtoy(tworker* w){
	if(w){
		tmsg_queue_destroy(w->queue);
		tfree(w);
	}
}
