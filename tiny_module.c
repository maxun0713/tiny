/*
 * tiny_module.c
 *
 *  Created on: 2014年3月17日
 *      Author: marv
 */


#include "tiny_module.h"
#include "tiny_assert.h"
#include "tiny_alloc.h"
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#define MAX_MODULE_TYPE 32

struct modules {
	int count;
	int lock;
	const char * path;
	struct tiny_module m[MAX_MODULE_TYPE];
};

struct modules* M = NULL;

void
tmodule_init(const char* path) {
	struct modules* m = talloc(sizeof(*m));
	m->count = 0;
	m->path = strdup(path);
	M = m;
}

void*
tmodule_inst_create(struct tiny_module* m){
	T_ERROR_PTR(m)
	T_ERROR_PTR(m->create)
	return m->create();
}


int
tmodule_inst_init(struct tiny_module* m, void* inst, const char * parm){
	T_ERROR_VAL(m)
	return m->init(inst, parm);
}


void
tmodule_inst_release(struct tiny_module* m, void* inst){
	T_ERROR_VOID(m);
	T_ERROR_VOID(m->release)
	m->release(inst);
}
