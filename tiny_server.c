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
#include "tiny_worker.h"
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


struct socket {
	int fd;
	int type;
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

static void
_init_conn(struct socket* slot, int fd, int type) {
	T_ERROR_VOID(slot)
	slot->fd =  fd;
	slot->type = type;
	slot->head = NULL;
	slot->tail = NULL;
	slot->token = time(NULL);
	slot->wb_size = 0;
	//TODO free buffer
}

static int
_reset_conn(struct socket* ctx){
	T_ERROR_VAL(ctx)
	if(ctx->fd>0){
		sp_release(ctx->fd);
	}
	ctx->fd = -1;
	ctx->token = 0;
	ctx->type = SOCKET_TYPE_CLOSED;

	struct write_buffer* buffer_ptr = ctx->head;
	struct write_buffer* p;
	while(buffer_ptr ){
		p = buffer_ptr;
		buffer_ptr = buffer_ptr->next;
		if(p->buffer){
			tfree(p->buffer);
		}
		tfree(p);
		ctx->wb_size --;
	}

	ctx->head = ctx->tail = NULL;
	T_ERROR_VAL(ctx->wb_size==0)

	return TINY_OK;
}

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
	tlog(LOG_LEVEL_ERROR, "tinyserver start listening %s:%d", addr,port);

	_init_conn(&server_ptr->slot[server_ptr->listenfd],
			server_ptr->listenfd,
			SOCKET_TYPE_LISTEN);

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
		sp_release(ptr->recvctrl_fd);
		sp_release(ptr->sendctrl_fd);
		sp_release(ptr->epfd);
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

static int
_accept(int fd){
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	memset(&cliaddr, 0, sizeof(cliaddr));
	int ret = accept(fd, (struct sockaddr*)&cliaddr, &len);
	if(ret <=0) {
		tlog(LOG_LEVEL_ERROR, "error in accept conn:[%s]", strerror(errno));
	}else{
		sp_nonblocking(ret);
		tlog(LOG_LEVEL_RELEASE, "recv conn from %s", inet_ntoa(cliaddr.sin_addr));
	}

	return ret;
}


static int
_recv(struct socket* ctx){
	T_ERROR_VAL(ctx)
	//unsigned short len;
	int fd = ctx->fd;
	struct write_buffer* wb = talloc(sizeof(*wb));
	wb->buffer = talloc(1024);
	int n = recv(fd, wb->buffer, 1024, 0);
	if(n <= 0){
		tfree(wb->buffer);
		tfree(wb);
		return n;
	}

	if(n< 1024){
		wb->buffer[n]= '\0';
	}
	wb->sz = n+1;
	wb->next = NULL;
	send(fd, wb->buffer, n, 0);

	tworker_transfer_msg(workers[0], wb);

	return TINY_OK;
}


int
tserver_shutdown(){
	T_ERROR_VAL(S)
	int flag = TINY_OK;
	int i;
	if(!sp_invalid(S->epfd)){
		sp_release(S->epfd);
	}

	if(!sp_invalid(S->listenfd)){
		sp_release(S->listenfd);
	}

	for(i = 0; i< MAX_SOCKET; i++){
		if(_reset_conn(&S->slot[i])){
			flag = TINY_ERROR;
		}
	}

	return flag;
}


int
tserver_poll(){
	T_ERROR_VAL(S)
	struct event* ev_ptr = NULL;
	struct socket* sock_ctx_ptr = NULL;
	int fd;
	int i,ret;
	int n = sp_wait(S->epfd, S->ev, MAX_EVENT);

	if(n <0){
		if(errno == EINTR){
			return TINY_OK;
		} else
		{
			tlog(LOG_LEVEL_ERROR ,"sp_wait error:%s",strerror(errno));
			return TINY_ERROR;
		}
	}

	for(i = 0; i< n; i++){
		ev_ptr = &(S->ev[i]);
		sock_ctx_ptr = (struct socket*)ev_ptr->s;
		fd = sock_ctx_ptr->fd;
		if(fd == S->listenfd) {
			ret = _accept(fd);
			if(ret >0 ) {
				_init_conn(&S->slot[ret], ret, SOCKET_TYPE_CONN);
				if(sp_add(S->epfd, ret, &S->slot[ret])){
					tlog(LOG_LEVEL_ERROR, "error in sp_add:[%s]", strerror(errno));
					_reset_conn(sock_ctx_ptr);
				}
			}
		} else if(fd == S->recvctrl_fd){
			//TODO SERVER_CMD
		} else {
			if(_recv(sock_ctx_ptr) <= 0)
			{
				sp_del(S->epfd, fd);
				_reset_conn(sock_ctx_ptr);
			}
		}
	}

	return n;
}
