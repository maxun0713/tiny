/*
 * service_gate.c
 *
 *  Created on: 2014年3月17日
 *      Author: marv
 */


#include "service_gate.h"
#include "../tiny_types.h"
#include <stdio.h>

void*
gate_create(void){
	return NULL;
}


int gate_init(void* inst, const char * parm){
	return 0;
}


void* gate_handle(void* arg){
	struct write_buffer* buffer = (struct write_buffer*)arg;
	printf("%s\n", (char*)buffer->buffer);
	return NULL;
}


void gate_release(void * inst){
	return ;
}
