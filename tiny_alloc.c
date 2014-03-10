/*
 * tiny_alloc.c
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#include <stdlib.h>
#include <string.h>

void*
talloc(size_t size){
	return malloc(size);
}

void*
trelloac(void* ptr ,size_t size) {
	return realloc(ptr, size);
}

void*
tcalloc(size_t size){
	void *ptr = talloc(size);
	if(ptr){
		memset(ptr, 0, size);
		return ptr;
	}

	return NULL;
}

void
tfree(void* ptr){
	free(ptr);
}
