/*
 * tiny_types.h
 *
 *  Created on: 2014年3月17日
 *      Author: marv
 */

#ifndef TINY_TYPES_H_
#define TINY_TYPES_H_

struct write_buffer {
	struct write_buffer * next;
	int sz;
	char* buffer;
};

#endif /* TINY_TYPES_H_ */
