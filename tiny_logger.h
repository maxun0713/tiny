/*
 * tiny_logger.h
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#ifndef TINY_LOGGER_H_
#define TINY_LOGGER_H_

typedef struct tiny_logger logger;



int  tlogger_init(const char* path, int level);
void tlogger_destroy(logger* ptr);
void tlog(logger* l, int level, const char* format,...);

extern logger* L;
#endif /* TINY_LOGGER_H_ */
