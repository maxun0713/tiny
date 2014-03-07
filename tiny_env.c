/*
 * tiny_env.c
 *
 *  Created on: 2014年3月5日
 *      Author: marv
 */


#include "tiny_env.h"
#include "tiny_assert.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

struct csp_env {
	lua_State* l;
};

struct csp_env* E = NULL;

void
init_env(){
	E = malloc(sizeof(*E));
	E->l = luaL_newstate();
}

int
set_env(const char* key, const char *val){
	lua_getglobal(E->l, key);
	T_ERROR_VAL(!lua_isnil(E->l, -1))
	lua_pop(E->l, 1);

	lua_pushstring(E->l, val);
	lua_setglobal(E->l, key);
	return 0;
}

const char*
get_env(const char *key){
	const char* val;

	lua_getglobal(E->l, key);
	val = lua_tostring(E->l, -1);
	lua_pop(E->l, 1);
	return val;
}


int
load_config(const char* script) {
	T_ERROR_VAL(script);
	int err = luaL_dofile(E->l, script);

	if(err){
		fprintf(stderr, "load config file[%s]failed:[%s]\n", script, lua_tostring(E->l, -1));
		lua_close(E->l);
		return TINY_ERROR;
	}

	lua_pushglobaltable(E->l);
	lua_pushnil(E->l);

	while(lua_next(E->l, -2) != 0){
		int tkey = lua_type(E->l, -2);
		if(tkey != LUA_TSTRING){
			fprintf(stderr, "parse config failed\n");
			exit(1);
		}
		const char *key = lua_tostring(E->l, -2);

		if(lua_type(E->l, -1) == LUA_TBOOLEAN){
			int b = lua_toboolean(E->l, -1);
			set_env(key, b? "true" : "false");
		}else {
			const char * val = lua_tostring(E->l, -1);
			set_env(key, val);
		}
		lua_pop(E->l, 1);
	}
	lua_pop(E->l, 1);

	return TINY_OK;
}

int
optint(const char *key, int opt){
	const char *val = get_env(key);
	if(val == NULL){
		char tmp[16];
		sprintf(tmp, "%d", opt);
		set_env(key, tmp);
		return opt;
	}

	return opt;
}

const char*
optstring(const char *key, const char *opt){
	const char *val = get_env(key);
	if(val == NULL){
		if(opt){
			set_env(key, opt);
		}
	}

	return val;
}

bool
optbool
(const char *key, bool opt){
	const char *val = get_env(key);
	if(val == NULL){
		if(opt){
			set_env(key, "true");
		}else{
			set_env(key, "false");
		}
		return opt;
	}

	return (strcmp(val, "true") == 0);
}
