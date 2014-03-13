/*
 * tiny_server.c
 *
 *  Created on: 2014年3月6日
 *      Author: marv
 */

#include "tiny_server.h"
#include "tiny_alloc.h"
#include "tiny_assert.h"
#include "tiny_logger.h"
#include "socket_poll.h"
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

#define MAX_EVENT 1024
#define MAX_SOCKET 102400
#define DEFAULT_BUF_SIZE 0xfffff

struct write_buffer {
	struct write_buffer * next;
	char *ptr;
	int sz;
	void *buffer;
};

struct socket {
	int fd;
	int type;
	int size;
	time_t token;
	int64_t wb_size;
	uintptr_t opaque;
	struct write_buffer * head;
	struct write_buffer * tail;
};

struct tiny_server {
	poll_fd epfd;
	poll_fd listenfd;

	int recvctrl_fd;
	int sendctrl_fd;

	struct event  ev[MAX_EVENT];
	struct socket slot[MAX_SOCKET];
	char   buffer[DEFAULT_BUF_SIZE];
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
		struct request_start reg;
	}  cmd;
};



struct tiny_server* S = NULL;


int
tserver_start_listen(struct tiny_server* server_ptr, short port, const char *addr){
	T_ERROR_VAL(addr)
	T_ERROR_VAL(server_ptr);
	T_ERROR_VAL((server_ptr->listenfd = socket(AF_INET, SOCK_STREAM, 0)) >0)
	sp_nonblocking(server_ptr->listenfd);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(addr);
	T_ERROR_VAL(bind(server_ptr->listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == 0)

	const int flag = 1;
	struct linger ling = {0, 0};
	T_ERROR_VAL(setsockopt(server_ptr->listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == 0)
	T_ERROR_VAL(setsockopt(server_ptr->listenfd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) == 0)
	T_ERROR_VAL(setsockopt(server_ptr->listenfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) == 0)

	T_ERROR_VAL(listen(server_ptr->listenfd, 1024) == 0)

	server_ptr->slot[server_ptr->listenfd].fd =  server_ptr->listenfd;
	server_ptr->slot[server_ptr->listenfd].type = SOCKET_TYPE_LISTEN;
	server_ptr->slot[server_ptr->listenfd].head = NULL;
	server_ptr->slot[server_ptr->listenfd].tail = NULL;
	server_ptr->slot[server_ptr->listenfd].token = time(NULL);
	return TINY_OK;
}


int
tserver_init(short port, const char* addr){
	struct tiny_server* ptr = talloc(sizeof(*ptr));
	int pipes[2];
	int ret;

	T_ERROR_VAL(pipe(pipes) == TINY_OK)
	ptr->recvctrl_fd = pipes[0];
	ptr->sendctrl_fd = pipes[1];
	ptr->epfd = sp_create();

	ret = tserver_start_listen(ptr, port, addr);
	if(ret ||
		sp_add(ptr->epfd, ptr->listenfd, &ptr->slot[ptr->listenfd])){
		close(ptr->recvctrl_fd);
		close(ptr->sendctrl_fd);
		close(ptr->epfd);
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

int _accept(int fd){
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	memset(&cliaddr, 0, sizeof(cliaddr));
	int ret = accept(fd, (struct sockaddr*)&cliaddr, &len);
	if(ret <=0) {
		tlog(LOG_LEVEL_ERROR, "error in accept conn:[%s]", strerror(errno));
	}else{
		tlog(LOG_LEVEL_RELEASE, "recv conn from %s", inet_ntoa(cliaddr.sin_addr));
	}

	return ret;
}

int tserver_poll(){
	int n = sp_wait(S->epfd, S->ev, MAX_EVENT);
	struct event* ev_ptr = NULL;
	struct socket* sock_ctx_ptr = NULL;
	int fd;
	int i;
	for(i = 0; i< n; i++){
		ev_ptr = &(S->ev[i]);
		sock_ctx_ptr = (struct socket*)ev_ptr->s;
		fd = sock_ctx_ptr->fd;
		if(fd == S->listenfd) {//TODO ACCEPT
			continue;
		} if(fd == S->recvctrl_fd){  //TODO SERVER_CMD
			continue;
		} else {
			//TODO handle msg here
		}
	}

	return n;
}
