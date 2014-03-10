/*
 * tiny_env.h
 *
 *  Created on: 2014年1月25日
 *      Author: marv
 */

#ifndef TINY_ENV_H_
#define TINY_ENV_H_

const char* tiny_getenv(const char *key);

void tiny_setenv(const char *key, const char *val);

void tiny_env_init();


#endif /* TINY_ENV_H_ */
