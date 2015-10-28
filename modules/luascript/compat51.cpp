/*************************************************************************/
/*  compat51.cpp                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifdef LUASCRIPT_ENABLED

#include "lua_script.h"

static bool env_init = false;

static void _get_env_table(lua_State *L) {

	LUA_MULTITHREAD_GUARD();
	lua_pushstring(L, "gdlua_envbox");
	if(env_init) {
		lua_rawget(L, LUA_REGISTRYINDEX);
		return;
	}
	/* create object ptr -> env mapping table */
	lua_newtable(L);
	/* make weak value metatable for ubox table to allow userdata to be
	   garbage-collected */
	lua_newtable(L);
	lua_pushliteral(L, "__mode");
	lua_pushliteral(L, "k");
	lua_rawset(L, -3);			   /* stack: string ubox mt */
	lua_setmetatable(L, -2);  /* stack: string ubox */
	lua_rawset(L, LUA_REGISTRYINDEX);
	env_init = true;

	_get_env_table(L);
}

void lua_setfenv(lua_State *L, int idx) {

	LUA_MULTITHREAD_GUARD();
	lua_pushvalue(L, idx); // [-1] = key, [-2] = value
	luaL_checktype(L, -1, LUA_TUSERDATA);

	_get_env_table(L); // [-1] = envbox [-2] = key [-3] = value
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_insert(L, -3); // [-1] = key [-2] = value [-3] = envbox
	lua_insert(L, -2); // [-1] = value [-2] = key [-3] = envbox
	lua_rawset(L, -3); // [-1] = envbox
	lua_pop(L, 1);
}

void lua_getfenv(lua_State *L, int idx) {

	LUA_MULTITHREAD_GUARD();
	lua_pushvalue(L, idx); // [-1] = key
	luaL_checktype(L, -1, LUA_TUSERDATA);

	_get_env_table(L); // [-1] = envbox [-2] = key
	luaL_checktype(L, -1, LUA_TTABLE);

	lua_insert(L, -2); // [-1] = key [-2] = envbox
	lua_rawget(L, -2); // [-1] = value [-2] = envbox
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_remove(L, -2); // [-1] = value
}

#endif // LUASCRIPT_ENABLED
