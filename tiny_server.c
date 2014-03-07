/*
 * tiny_server.c
 *
 *  Created on: 2014年3月6日
 *      Author: marv
 */

#include "tiny_server.h"
#include "tiny_alloc.h"
#include "tiny_assert.h"

#define MAX_EVENT 1024
#define DEFAULT_BUF_SIZE 0xfffff

struct tiny_server {
	poll_fd epfd;

	int recvctrl_fd;
	int sendctrl_fd;

	struct event* ev;
	char*  send_buf;
	char*  recv_buf;
};

struct tiny_server* S = NULL;

int
tserver_init(){
	struct tiny_server* ptr = talloc(sizeof(*ptr));
	int pipes[2];

	T_ERROR_VAL(pipe(pipes) == TINY_OK)
	ptr->recvctrl_fd = pipes[0];
	ptr->sendctrl_fd = pipes[1];
	ptr->ev = talloc(sizeof(struct event) * MAX_EVENT);

	ptr->send_buf = talloc(DEFAULT_BUF_SIZE);
	ptr->recv_buf = talloc(DEFAULT_BUF_SIZE);
	return TINY_OK;
}

int
tserver_command(){

}
