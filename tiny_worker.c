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


struct tiny_worker*
tworker_new(msg_handler handler, struct tiny_worker_parm* wp){
	T_ERROR_PTR(handler)
	T_ERROR_PTR(wp)

	struct tiny_worker* w = talloc(sizeof(*w));
	T_ERROR_PTR(w)
	w->handler = handler;
	w->id = -1;
	w->starttime = -1;
	w->status = WORKER_STATUS_IDLE;
	w->wp = wp;
	w->lock = 0;
	w->head = w->tail = NULL;
	return w;
}


void* _work(void* arg){
	struct tiny_worker* w= (struct tiny_worker*)arg;
	T_ERROR_PTR(w);
	while(__sync_lock_test_and_set(&w->lock,1)) {}   //insure lock is modified
	__sync_lock_release(&w->lock);


//	pthread_mutex_lock( &m_stMutex );
//
//	while( IsToBeBlocked() || m_iRunStatus == rt_stopped )
//	{
//		if( m_iRunStatus == rt_stopped )
//		{
//			pthread_mutex_unlock( &m_stMutex );
//			Release();
//			pthread_exit( (void *)m_abyRetVal );
//		}
//
//		m_iRunStatus = rt_blocked;
//		pthread_cond_wait( &m_stCond, &m_stMutex );
//	}
//
//	m_iRunStatus = rt_running;
//
//	pthread_mutex_unlock( &m_stMutex );
	while(w->status == WORKER_STATUS_RUNNING){
		pthread_mutex_lock(&w->wp->mutex);
		pthread_cond_wait(&w->wp->mutex, &w->wp->cond);
		//Fetch msg

		pthread_mutex_unlock(&w->wp->mutex);
	}

	pthread_exit(NULL);
	return NULL;
}


int tworker_run(struct tiny_worker* w){
	T_ERROR_VAL(w)
	T_ERROR_VAL(w->wp)
	T_ERROR_VAL(w->status != WORKER_STATUS_RUNNING)

	while(__sync_lock_test_and_set(&w->lock,1)) {}
	int ret = pthread_create(&w->id, NULL, _work, w);
	if(ret == 0){
		w->starttime = time(NULL);
		w->status = WORKER_STATUS_RUNNING;
	}
	__sync_lock_release(&w->lock);

	return ret;
}
int	tworker_stop(struct tiny_worker* w){
	while(__sync_lock_test_and_set(&w->lock,1)) {}
	w->status = WORKER_STATUS_STOPPED;
	__sync_lock_release(&w->lock);

	void** status = NULL;
	pthread_join(w->id, status);
	return 0;
}
