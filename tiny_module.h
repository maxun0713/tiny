/*
 * tiny_module.h
 *
 *  Created on: 2014年3月17日
 *      Author: marv
 */

#ifndef TINY_MODULE_H_
#define TINY_MODULE_H_

typedef void * (*tiny_dl_create)(void);
typedef int (*tiny_dl_init)(void * inst, const char * parm);
typedef void * (*tiny_dl_handler)(void * arg);
typedef void (*tiny_dl_release)(void * inst);

struct tiny_module {
	const char * name;
	void * module;
	tiny_dl_create create;
	tiny_dl_init init;
	tiny_dl_handler handle;
	tiny_dl_release release;
};

void tmodule_init(const char* path);



void* tmodule_inst_create(struct tiny_module* m);
int tmodule_inst_init(struct tiny_module* m, void* inst, const char * parm);
void tmodule_inst_release(struct tiny_module* m, void* inst);


#endif /* TMODULE_H_ */
