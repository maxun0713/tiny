/*
 * tiny_env.h
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#ifndef TINY_ENV_H_
#define TINY_ENV_H_
#include <stdbool.h>

const char* get_env(const char *key);
int set_env(const char* key, const char *val);

void init_env();

int load_config(const char* script);
bool optbool(const char *key, bool opt);
const char* optstring(const char *key, const char *opt);
int optint(const char *key, int opt);

#endif /* TINY_ENV_H_ */
