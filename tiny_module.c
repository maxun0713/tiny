/*
 * tiny_module.c
 *
 *  Created on: 2014年3月17日
 *      Author: marv
 */


#include "tiny_module.h"
#include "tiny_assert.h"
#include "tiny_alloc.h"
#include "tiny_logger.h"
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#define MAX_MODULE_TYPE 32
#define MAX_MODULE_PATH_LENGTH 1024

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
	m->lock = 0;
	M = m;
}


struct tiny_module*
_query(const char* name){
	int i;
	T_ERROR_PTR(name)
	for(i=0; i<M->count; i++){
		if(strcmp(M->m[i].name, name) == 0){
			return &M->m[i];
		}
	}

	return NULL;
}


void*
_try_open(struct modules* m, const char* name){
	T_ERROR_PTR(m)
	T_ERROR_PTR(m->path)
	T_ERROR_PTR(name)

	char tmp[MAX_MODULE_PATH_LENGTH];
	int n = snprintf(tmp, MAX_MODULE_PATH_LENGTH, "%s/lib%s.so", m->path, name);
	T_ERROR_PTR(n < MAX_MODULE_PATH_LENGTH)
	tmp[n] = '\0';

	void* dl = dlopen(tmp, RTLD_NOW | RTLD_GLOBAL);
	if (dl == NULL) {
		tlog(LOG_LEVEL_ERROR, "try open %s failed : %s\n",tmp,dlerror());
	}

	return dl;
}


int
_open_sym(struct tiny_module* mod){
	T_ERROR_VAL(mod)
	size_t name_size = strlen(mod->name);
	char tmp[name_size + 9]; // create/init/release/handle , longest name is release (7)
	memcpy(tmp, mod->name, name_size);
	strcpy(tmp+name_size, "_create");
	mod->create = dlsym(mod->module, tmp);
	strcpy(tmp+name_size, "_init");
	mod->init = dlsym(mod->module, tmp);
	strcpy(tmp+name_size, "_handle");
	mod->handle = dlsym(mod->module, tmp);
	strcpy(tmp+name_size, "_release");
	mod->release = dlsym(mod->module, tmp);

	return mod->init == NULL;
}


struct tiny_module*
tmodule_query(const char* name){
	T_ERROR_PTR(M)
	T_ERROR_PTR(name)
	struct tiny_module* mod = _query(name);
	if(mod){
		return mod;
	}

	while(__sync_lock_test_and_set(&M->lock,1)) {}
	void* dl = _try_open(M, name);
	if(dl){
		int index = M->count;
		M->m[index].name = name;
		M->m[index].module = dl;
		if(_open_sym(&M->m[index]) == 0){
			M->m[index].name = strdup(name);
			mod = &M->m[index];
			M->count++;
		}
	}

	__sync_lock_release(&M->lock);
	return mod;
}


void*
tmodule_inst_create(struct tiny_module* mod){
	T_ERROR_PTR(mod)
	T_ERROR_PTR(mod->create)
	return mod->create();
}


int
tmodule_inst_init(struct tiny_module* mod, void* inst, const char * parm){
	T_ERROR_VAL(mod)
	return mod->init(inst, parm);
}


void
tmodule_inst_release(struct tiny_module* mod, void* inst){
	T_ERROR_VOID(mod);
	T_ERROR_VOID(mod->release)
	mod->release(inst);
}
