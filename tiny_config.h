/*
 * tiny_config.h
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */

#ifndef TINY_CONFIG_H_
#define TINY_CONFIG_H_

struct tiny_config {
	int daemonlized;
	int nthread;
	char* logpath;
};

struct tiny_config config;
#endif /* TINY_CONFIG_H_ */
