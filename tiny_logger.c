/*
 * tiny_logger.c
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */


#include "tiny_logger.h"
#include "tiny_constdef.h"
#include "tiny_alloc.h"
#include "tiny_assert.h"
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>


struct tiny_logger {
	char* path;
	FILE* fp;
	int   level;
};

static const char* LOGGER_MARK[] = {
	NULL,
	"RELEASE",
	"ERROR",
	"DEBUG"
};

logger* L = NULL;

logger*
tlogger_new(const char* path, int level){
	T_ERROR_PTR(path)
	logger* ptr =(logger*)talloc(sizeof(logger));
	ptr->fp = fopen(path, "w");
	if(!ptr->fp) {
		tfree(ptr);
		return NULL;
	}
	ptr->level = level;
	ptr->path = strdup(path);
	return ptr;
}

void
tlogger_destroy(logger* ptr){
	T_ERROR_VOID(ptr)
	T_ERROR_VOID(ptr->fp)
	T_ERROR_VOID(ptr->path)
	tfree(ptr->path);
	fclose(ptr->fp);
	tfree(ptr);
}


int
tlogger_init(const char* path, int level){
	logger* ptr = tlogger_new(path, level);
	if(ptr){
		L = ptr;
		return TINY_OK;
	}

	return TINY_ERROR;
}

void
tlog(logger* l, int level, const char* format,...){
	T_ERROR_VOID(l)

	if(level >= l->level){
		va_list args;
		va_start(args, format);
		va_end(args);

		time_t t;
		time(&t);
		T_ERROR_VOID(l->fp)
		fprintf(l->fp, "[%s][%s]", LOGGER_MARK[level],ctime(&t));
		vfprintf(l->fp, format, args);
		fprintf(l->fp, "\n");
	}
}
