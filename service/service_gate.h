/*
 * service_gate.h
 *
 *  Created on: 2014年3月17日
 *      Author: marv
 */

#ifndef SERVICE_GATE_H_
#define SERVICE_GATE_H_


void* gate_create(void);
int gate_init(void* inst, const char * parm);
void* gate_handle(void* arg);
void  gate_release(void * inst);

#endif /* SERVICE_GATE_H_ */
