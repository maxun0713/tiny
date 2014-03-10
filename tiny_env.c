/*
 * tiny_env.c
 *
 *  Created on: 2014年1月25日
 *      Author: marv
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

struct tiny_env{
	lua_State *L;
	int lock;
};


#define LOCK(q) while (__sync_lock_test_and_set(&(q)->lock,1)) {}
#define UNLOCK(q) __sync_lock_release(&(q)->lock);

static struct tiny_env *E = NULL;

const char*
tiny_getenv(const char *key){
	LOCK(E)

	lua_State *L = E->L;
	const char* val;

	lua_getglobal(L, key);
	val = lua_tostring(L, -1);
	lua_pop(L, 1);

	UNLOCK(E)

	return val;
}

void
tiny_setenv(const char *key, const char *val) {
	LOCK(E)

	lua_State *L = E->L;
	lua_getglobal(L, key);
	assert(lua_isnil(L, -1));
	lua_pop(L, 1);

	lua_pushstring(L, val);
	lua_setglobal(L, key);

	UNLOCK(E)
}


void
tiny_env_init(){
	E = malloc(sizeof(*E));
	E->L = luaL_newstate();
	E->lock = 0;
}
