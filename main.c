/*
 * main.c
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */


#include "tiny_env.h"
#include "tiny_config.h"
#include "tiny_assert.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int
main(int argc, char** argv)
{
	int ch;
	const char * config_file = "config";

	while((ch = getopt(argc, argv, "c:hr")) != -1)
	{
		switch(ch)
		{
		case 'c':
			config_file = optarg;
			break;
		case 'h':
			fprintf(stdin,"reload...\n");  //TODO
			break;
		case 'r':
			fprintf(stdin,"reload...\n");  //TODO
			exit(1);
		default:
			fprintf(stderr,"unknown opt %c\n", ch);
			exit(1);
		}
	}

	init_env();

	T_ERROR_VAL(load_config(config_file)==TINY_OK);
	config.daemonlized = optint("daemon", 0);
	config.nthread = optint("worker", 2);
	config.logpath = optstring("logpath", argv[0]);

	start();

	exit(0);
}
