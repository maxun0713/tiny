/*
 * tiny_alloc.h
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#ifndef TINY_ALLOC_H_
#define TINY_ALLOC_H_

#include <stddef.h>
//TODO consider mem pool later

void* talloc(size_t size);
void* trelloac(void* ptr ,size_t size);
void* tcalloc(size_t size);
void  tfree(void* ptr);


#endif /* TINY_ALLOC_H_ */
