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
	"WARN",
	"ERROR",
	"DEBUG"
};

logger* L = NULL;

logger*
tlogger_new(const char* path, int level){
	T_ERROR_PTR(path)
	logger* ptr =(logger*)talloc(sizeof(logger));
	char tmp[1024];
	int n = snprintf(tmp, sizeof(tmp), "%stserver.log", path);
	if(n < sizeof(tmp)-1){
		tmp[n] = '\0';
	}else{
		tmp[sizeof(tmp)-1] = '\0';
	}

	ptr->fp = fopen(tmp, "w");
	if(!ptr->fp) {
		tfree(ptr);
		return NULL;
	}
	ptr->level = level;
	ptr->path = strdup(tmp);
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
tlogger_release(){
	tlogger_destroy(L);
}


void
tlog(int level, const char* format,...){
	T_ERROR_VOID(L)
	T_ERROR_VOID(L->fp)

	FILE* fp = L->fp;
	char tmp[64];
	if(level>0 && level <= L->level){
		va_list args;
		va_start(args, format);
		va_end(args);

		time_t t;
		time(&t);
		struct tm* curr = localtime(&t);
		int n = strftime(tmp, sizeof(tmp), "%D/%H:%M:%S", curr);
		tmp[n] = '\0';
		fprintf(fp, "[%s][%s]", tmp, LOGGER_MARK[level]);
		vfprintf(fp, format, args);
		fprintf(fp, "\n");
	}
}
