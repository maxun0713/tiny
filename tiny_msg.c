/*
 * tiny_msg.c
 *
 *  Created on: 2014年3月19日
 *      Author: marv
 */

#include "tiny_msg.h"
#include "tiny_alloc.h"
#include "tiny_assert.h"
#include <stdbool.h>

#define LOCK(q) while (__sync_lock_test_and_set(&(q)->lock,1)) {}
#define UNLOCK(q) __sync_lock_release(&(q)->lock);

struct tmsg_queue {
	int cap;
	int tail;
	int head;
	int lock;
	int locksession;
	tmsg* queue;
};


tmsg_queue*
tmsg_queue_new() {
	tmsg_queue* q = talloc(sizeof(*q));
	T_ERROR_PTR(q)
	q->cap = DEFAULT_QUEUE_SIZE;
	q->head = 0;
	q->tail = 0;
	q->lock = 0;
	q->locksession = 0;
	q->queue = talloc(sizeof(tmsg) * DEFAULT_QUEUE_SIZE);

	return q;
}

void
_expend_queue(tmsg_queue* q) {
	tmsg* new_queue = talloc(sizeof(tmsg) * q->cap * 2);
	int i;
	for(i = 0; i<q->cap; i++){
		new_queue[i] = q->queue[(q->head++)% q->cap];
	}

	q->head = 0;
	q->tail = q->cap;
	q->cap = q->cap * 2;
	tfree(q->queue);
	q->queue = new_queue;
}


int
tmsg_queue_push(tmsg_queue* q, tmsg* msg){
	T_ERROR_VAL(q)
	T_ERROR_VAL(msg)

	LOCK(q)
	q->queue[q->tail++] = *msg;
	if(q->tail == q->cap){
		q->tail = 0;
	}
	if(q->tail == q->head){
		_expend_queue(q);
	}

	UNLOCK(q)
	return 0;
}


int
tmsg_queue_pop(tmsg_queue* q, tmsg* msg){
	T_ERROR_VAL(q)

	LOCK(q)
	int ret = TINY_ERROR;

	if(q->head != q->tail){
		*msg = q->queue[q->head];
		ret = TINY_OK;
	}

	if(++q->head == q->cap){
		q->head = 0;
	}

	UNLOCK(q)
	return ret;
}


void
tmsg_queue_destroy(tmsg_queue* q){
	if(!q){
		return;
	}

	int i;
	tmsg* msg;
	for(i = 0; i< q->cap; i++){
		msg = &q->queue[i];
		tfree(msg->data);
	}
	tfree(q->queue);
	tfree(q);
}


bool
tmsg_queue_empty(tmsg_queue* q){
	if(!q){
		return false;
	}

	LOCK(q)
	bool b = (q->head == q->tail);
	UNLOCK(q)

	return b;
}

