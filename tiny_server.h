/*
 * tiny_server.h
 *
 *  Created on: 2014年3月6日
 *      Author: marv
 */

#ifndef TINY_SERVER_H_
#define TINY_SERVER_H_

#include "socket_poll.h"

int tserver_init(short port, const char* addr);
int tserver_send_command();


#endif /* TINY_SERVER_H_ */
