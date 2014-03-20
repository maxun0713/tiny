/*
 * tiny_constdef.h
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#ifndef TINY_CONSTDEF_H_
#define TINY_CONSTDEF_H_

#include <stdio.h>

#define TINY_OK 0
#define TINY_ERROR -1

#define LOG_LEVEL_RELEASE 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_DEBUG 4

#define SOCKET_TYPE_MIN
#define SOCKET_TYPE_CLOSED 0
#define SOCKET_TYPE_LISTEN 1
#define SOCKET_TYPE_CONN 2

#define SOCKET_TYPE_MAX

enum WORKER_STATUS {
	WORKER_STATUS_IDLE,
	WORKER_STATUS_RUNNING,
	WORKER_STATUS_STOPPED,
};

#define DEFAULT_QUEUE_SIZE 1024

#endif /* TINY_CONSTDEF_H_ */
