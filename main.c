/*
 * main.c
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */


#include "tiny_env.h"
#include "tiny_config.h"
#include "tiny_assert.h"
#include "tiny_start.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdbool.h>
#include <string.h>

void
_env_init(lua_State *L){
	lua_pushglobaltable(L);
	lua_pushnil(L);

	while(lua_next(L, -2) != 0){
		int tkey = lua_type(L, -2);
		if(tkey != LUA_TSTRING){
			fprintf(stderr, "parse config failed\n");
			exit(1);
		}
		const char *key = lua_tostring(L, -2);

		if(lua_type(L, -1) == LUA_TBOOLEAN){
			int b = lua_toboolean(L, -1);
			tiny_setenv(key, b? "true" : "false");
		}else {
			const char * val = lua_tostring(L, -1);
			tiny_setenv(key, val);
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

int
_env_optint(const char *key, int opt){
	const char *val = tiny_getenv(key);
	if(val == NULL){
		char tmp[16];
		sprintf(tmp, "%d", opt);
		tiny_setenv(key, tmp);
		return opt;
	}

	return atoi(val);
}

const char*
_env_optstring(const char* key, const char* opt){
	const char *val = tiny_getenv(key);
	if(val == NULL){
		if(opt){
			tiny_setenv(key, opt);
			return opt;
		}
	}

	return val;
}

bool
_env_optbool(const char* key, bool opt){
	const char *val = tiny_getenv(key);
	if(val == NULL){
		if(opt){
			tiny_setenv(key, "true");
		}else{
			tiny_setenv(key, "false");
		}
		return opt;
	}

	return (strcmp(val, "true") == 0);
}

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

	tiny_env_init();

	struct lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	lua_close(L);


	L = luaL_newstate();
	int err = luaL_dofile(L, config_file);
	if(err){
		fprintf(stderr, "load config file[%s]failed:[%s]\n", config_file, lua_tostring(L, -1));
		lua_close(L);
		exit(-1);
	}

	_env_init(L);
	config.daemonlized = _env_optint("daemon", 0);
	config.nthread = _env_optint("worker", 2);
	config.logpath = _env_optstring("logpath", "./");
	config.port = _env_optint("port", 27017);
	config.addr = _env_optstring("addr", "127.0.0.1");

	start();

	exit(0);
}
