/*
 * tiny_msg.h
 *
 *  Created on: 2014年3月19日
 *      Author: marv
 */

#ifndef TINY_MSG_H_
#define TINY_MSG_H_

#include "tiny_types.h"
#include <stdbool.h>
#include <time.h>

typedef struct msg_header tmsg_header;
typedef struct tmsg tmsg;
typedef struct tmsg_queue tmsg_queue;

struct msg_header {
	int	   type; //SOCKET,SYSTEM...
	int	   index; //for socket msg means socketfd or means session id
	union {
		time_t timestamp; //for socket msg
		char dummy[sizeof(time_t)];
	} extra;

};

struct tmsg {
	tmsg_header header;
	void* data;
	size_t sz;
};


tmsg_queue* tmsg_queue_new();
int tmsg_queue_push(tmsg_queue* q, tmsg* msg);
int tmsg_queue_pop(tmsg_queue* q, tmsg* msg);
void tmsg_queue_destroy(tmsg_queue* q);
bool tmsg_queue_empty(tmsg_queue* q);

#endif /* TINY_MSG_H_ */
