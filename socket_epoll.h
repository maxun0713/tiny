/*
 * =====================================================================================
 *
 *       Filename:  socket_epoll.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年12月03日 14时54分56秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  marv (mx), marv0713@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef SOCKET_EPOLL_H
#define SOCKET_EPOLL_H

#include "socket_poll.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
static bool
sp_invalid(poll_fd fd){
    return fd == -1;
}

static poll_fd
sp_create(){
    return epoll_create(1024);
}

static void 
sp_release(poll_fd fd){
    close(fd);
}

static int 
sp_add(poll_fd fd, int sock, void *ud){
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = ud;
    return epoll_ctl(fd, EPOLL_CTL_ADD, sock, &ev);
}

static void 
sp_del(poll_fd fd, int sock){
    epoll_ctl(fd, EPOLL_CTL_DEL, sock, NULL);
}

static void 
sp_write(poll_fd fd, int sock, void *ud, bool enable){
    struct epoll_event ev;
    ev.events = EPOLLIN | (enable ? EPOLLOUT :0);
    ev.data.ptr = ud;
    epoll_ctl(fd, EPOLL_CTL_MOD, sock, &ev);
}

static int sp_wait(poll_fd fd, struct event *e, int max){
    struct epoll_event ev[max];
    int n = epoll_wait(fd, ev, max, -1);
    int i;
    for(i=0; i<n; i++){
        e[i].s = ev[i].data.ptr;
        unsigned flag = ev[i].events;
        e[i].write = (flag & EPOLLOUT) != 0;
        e[i].read  = (flag & EPOLLIN) != 0;
    }
    
    return n;
}

static void sp_nonblocking(int sock){
    int flag = fcntl(sock, F_GETFL, 0);
    if( flag !=0) return;

    fcntl(sock, F_SETFL, flag| O_NONBLOCK);
}

#endif
