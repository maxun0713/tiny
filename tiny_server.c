/*
 * tiny_server.c
 *
 *  Created on: 2014年3月6日
 *      Author: marv
 */

#include "tiny_server.h"
#include "tiny_alloc.h"
#include "tiny_assert.h"
#include "socket_poll.h"
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define MAX_EVENT 1024
#define DEFAULT_BUF_SIZE 0xfffff

struct tiny_server {
	poll_fd epfd;
	poll_fd listenfd;

	int recvctrl_fd;
	int sendctrl_fd;

	struct event* ev;
	char*  send_buf;
	char*  recv_buf;
};

struct request_start {
	char  addr[64];
	short port;
	bool block;
};

struct tiny_server_command {
	char header[8];
	union {
		char dummy[256];
		struct request_start;
	};
};



struct tiny_server* S = NULL;

int
tserver_init(short port, const char* addr){
	struct tiny_server* ptr = talloc(sizeof(*ptr));
	int pipes[2];
	int ret;

	T_ERROR_VAL(pipe(pipes) == TINY_OK)
	ptr->recvctrl_fd = pipes[0];
	ptr->sendctrl_fd = pipes[1];
	ptr->epfd = sp_create();
	ptr->ev = talloc(sizeof(struct event) * MAX_EVENT);
	ptr->send_buf = talloc(DEFAULT_BUF_SIZE);
	ptr->recv_buf = talloc(DEFAULT_BUF_SIZE);

	ret = tserver_start_listen(port, addr);
	if(ret){
		close(ptr->recv_buf);
		close(ptr->sendctrl_fd);
		close(ptr->epfd);
		tfree(ptr->ev);
		tfree(ptr->send_buf);
		tfree(ptr->recv_buf);
		tfree(ptr);
		return TINY_ERROR;
	}

	S = ptr;
	return TINY_OK;
}


int
tserver_send_command(char len, char type, struct tiny_server_command* cmd){
	T_ERROR_VAL(cmd)
	cmd->header[6] = len;
	cmd->header[7] = type;
	for(;;) {
		int n = write(S->sendctrl_fd, &cmd->header[6], len+2);
		if(n<0) {
			if(errno != EINTR){
				tlog(LOG_LEVEL_ERROR, "send server command failed[%s]", strerror(errno));
			}
			continue;
		}
		T_ERROR_VAL(n == len+2);
		break;
	}

	return TINY_OK;
}

int
tserver_start_listen(short port, const char *addr){
	T_ERROR_VAL(addr)
	T_ERROR_VAL(S);
	T_ERROR_VAL((S->listenfd = socket(AF_INET, SOCK_STREAM, 0)) >0)
	sp_nonblocking(S->listenfd);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr = inet_addr(addr);
	T_ERROR_VAL(bind(S->listenfd, &serv_addr, sizeof(serv_addr)) == 0)

	const int flag = 1;
	struct linger ling = {0, 0};
	T_ERROR_VAL(setsockopt(S->listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == 0)
	T_ERROR_VAL(setsockopt(S->listenfd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) == 0)
	T_ERROR_VAL(setsockopt(S->listenfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) == 0)

	T_ERROR_VAL(listen(S->listenfd, 1024))

	return TINY_OK;
}
