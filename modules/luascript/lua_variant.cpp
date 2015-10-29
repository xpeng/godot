/*************************************************************************/
/*  lua_script.cpp                                                       */
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

#include <stdlib.h>
#include <string.h>
#include "lua_script.h"
#include "globals.h"
#include "global_constants.h"
//#include "gd_compiler.h"
#include "os/file_access.h"

//////////////////////////////
//		 VARIANT            //
//////////////////////////////

typedef struct {
	const char *type;
	Variant::Type vt;
} BulitinTypes;

static BulitinTypes vtypes[] = {
	// math types
	{ "Vector2", Variant::VECTOR2 },
	{ "Rect2", Variant::RECT2 },
	{ "Vector3", Variant::VECTOR3 },
	{ "Matrix32", Variant::MATRIX32 },
	{ "Plane", Variant::PLANE },
	{ "Quat", Variant::QUAT },
	{ "AABB", Variant::_AABB },
	{ "Matrix3", Variant::MATRIX3 },
	{ "Transform", Variant::TRANSFORM },
	// misc types
	{ "Color", Variant::COLOR },
	{ "Image", Variant::IMAGE },
	{ "NodePath", Variant::NODE_PATH },
	{ "RID", Variant::_RID },
	{ "Object", Variant::OBJECT },
	{ "InputEvent", Variant::INPUT_EVENT },
	{ "Dictionary", Variant::DICTIONARY },
	{ "Array", Variant::ARRAY },
	{ "RawArray", Variant::RAW_ARRAY },
	{ "IntArray", Variant::INT_ARRAY },
	{ "FloatArray", Variant::REAL_ARRAY },
	{ "StringArray", Variant::STRING_ARRAY },
	{ "Vector2Array", Variant::VECTOR2_ARRAY },
	{ "Vector3Array", Variant::VECTOR3_ARRAY },
	{ "ColorArray", Variant::COLOR_ARRAY },
};

static Variant * luaL_checkvariant(lua_State *L, int idx) {

	void *ptr = luaL_checkudata(L, idx, "LuaVariant");
	return *((Variant **) ptr);
}


int LuaInstance::l_bultins_caller_wrapper(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	const char *key = luaL_checkstring(L, lua_upvalueindex(1));
	int top = lua_gettop(L);

	Variant* var = luaL_checkvariant(L, 1);
	Variant::CallError err;
	Variant ret;

	if(top == 1) {

		ret = var->call(key, NULL, 0, err);
	} else {

		Variant *vars = memnew_arr(Variant, top - 1);
		Variant *args[128];
		for(int idx = 2; idx <= top; idx++)
		{
			Variant& var = vars[idx - 2];
			args[idx - 2] = &var;
			l_get_variant(L, idx, var);
		}
		ret = var->call(key, (const Variant**) (args), top - 1, err);
		memdelete_arr(vars);
	}
	switch(err.error) {
	case Variant::CallError::CALL_OK:
		l_push_variant(L, ret);
		return 1;
	case Variant::CallError::CALL_ERROR_INVALID_METHOD:
		luaL_error(L, "Invalid method '%s'", key);
		break;
	case Variant::CallError::CALL_ERROR_INVALID_ARGUMENT:
		luaL_error(L, "Invalid argument to call '%s'", key);
		break;
	case Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS:
		luaL_error(L, "Too many arguments to call '%s'", key);
		break;
	case Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS:
		luaL_error(L, "Too few arguments to call '%s'", key);
		break;
	case Variant::CallError::CALL_ERROR_INSTANCE_IS_NULL:
		luaL_error(L, "Instance is null");
		break;
	}
	return 0;
}

int LuaInstance::l_bultins_wrapper(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Variant::Type type = (Variant::Type) lua_tointeger(L, lua_upvalueindex(1));
	const char *type_name = lua_tostring(L, lua_upvalueindex(2));
	int top = lua_gettop(L);
	Variant::CallError err;
	Variant ret;

	if(top <= 1) {

		ret = Variant::construct(type, NULL, 0, err);
	} else {

		Variant *vars = memnew_arr(Variant, top - 1);
		Variant *args[128];
		for(int idx = 2; idx <= top; idx++) {

			Variant& var = vars[idx - 2];
			args[idx - 2] = &var;
			l_get_variant(L, idx, var);
		}
		ret = Variant::construct(type, (const Variant**) (args), top - 1, err);
		memdelete_arr(vars);
	}
	switch(err.error) {
	case Variant::CallError::CALL_OK:
		l_push_variant(L, ret);
		return 1;
	case Variant::CallError::CALL_ERROR_INVALID_METHOD:
		luaL_error(L, "Invalid method");
		break;
	case Variant::CallError::CALL_ERROR_INVALID_ARGUMENT:
		luaL_error(L, "Invalid argument to construct built-in type '%s", type_name);
		break;
	case Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS:
		luaL_error(L, "Too many arguments to construct built-in type '%s", type_name);
		break;
	case Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS:
		luaL_error(L, "Too few arguments to construct built-in type '%s", type_name);
		break;
	case Variant::CallError::CALL_ERROR_INSTANCE_IS_NULL:
		luaL_error(L, "Instance is null");
		break;
	}
	return 0;
}

int LuaInstance::l_bultins_tostring(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	const char *type_name = lua_tostring(L, lua_upvalueindex(1));
	lua_pushfstring(L, "Bultin type '%s'", type_name);
	return 1;
}

int LuaInstance::l_bultins_index(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Variant::Type type = (Variant::Type) lua_tointeger(L, lua_upvalueindex(1));
	const char *type_name = lua_tostring(L, lua_upvalueindex(2));
	const char *key = luaL_checkstring(L, 2);

	if(Variant::has_numeric_constant(type, key)) {

		lua_pushnumber(L, Variant::get_numeric_constant_value(type, key));
		return 1;
	}
	luaL_error(L, "Bultin type '%s' has no constant: '%s'", type_name, key);
	return 0;
}

bool LuaInstance::l_register_bultins_ctors(lua_State *L)
{
	for(int idx = 0; idx < (sizeof(vtypes) / sizeof(vtypes[0])); idx++) {

		BulitinTypes& t = vtypes[idx];
		lua_newtable(L); 
		{
			lua_newtable(L);
			{
				lua_pushinteger(L, t.vt);
				lua_pushstring(L, t.type);
				lua_pushcclosure(L, l_bultins_wrapper, 2);
				lua_setfield(L, -2, "__call");

				lua_pushstring(L, t.type);
				lua_pushcclosure(L, l_bultins_tostring, 1);
				lua_setfield(L, -2, "__tostring");

				lua_pushinteger(L, t.vt);
				lua_pushstring(L, t.type);
				lua_pushcclosure(L, l_bultins_index, 2);
				lua_setfield(L, -2, "__index");
			}
			lua_setmetatable(L, -2);
		}
		lua_setglobal(L, t.type);
	}
	return true;
}

int LuaInstance::meta_bultins__gc(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Variant* var = luaL_checkvariant(L, 1);
	memdelete(var);

	lua_pushnil(L);
	lua_setmetatable(L, 1);

	return 0;
}

int LuaInstance::meta_bultins__evaluate(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Variant::Operator op = (Variant::Operator) lua_tointeger(L, lua_upvalueindex(1));

	Variant* var1 = luaL_checkvariant(L, 1);

	Variant var2;
	l_get_variant(L, 2, var2);

	Variant ret;
	bool valid = false;
	Variant::evaluate(op, *var1, var2, ret, valid);
	if(valid) {

		l_push_variant(L, ret);
		return 1;
	}
	return 0;
}

static int l_builtins_iterator(lua_State *L) {

	LUA_MULTITHREAD_GUARD();
	Variant& var = *luaL_checkvariant(L, 1);
	if(var.get_type() == Variant::DICTIONARY) {

		Variant k;
		const Variant *key = NULL;
		Dictionary& dict = var.operator Dictionary();
		if(!lua_isnil(L, 2)) {
			LuaInstance::l_get_variant(L, 2, k);
			key = &k;
		}
		key = dict.next(key);
		if(key != NULL) {
			LuaInstance::l_push_variant(L, *key);
			LuaInstance::l_push_variant(L, dict[*key]);
			return 2;
		}
	} else {

		int idx = 0;
		if(!lua_isnil(L, 2))
			idx = luaL_optinteger(L, 2, 0) + 1;

		bool r_valid = false;
		Variant value = var.get(idx, &r_valid);
		if(r_valid) {
			lua_pushinteger(L, idx);
			LuaInstance::l_push_variant(L, value);
			return 2;
		}
	}
	return 0;
}

int LuaInstance::meta_bultins__pairs(lua_State *L) {

	LUA_MULTITHREAD_GUARD();
	Variant& var = *luaL_checkvariant(L, 1);
	Variant::Type vt = var.get_type();
	switch(vt) {
	case Variant::DICTIONARY:
	case Variant::ARRAY:
	case Variant::RAW_ARRAY:
	case Variant::INT_ARRAY:
	case Variant::REAL_ARRAY:
	case Variant::STRING_ARRAY:
	case Variant::VECTOR2_ARRAY:
	case Variant::VECTOR3_ARRAY:
	case Variant::COLOR_ARRAY:
		lua_pushcclosure(L, l_builtins_iterator, 0);
		LuaInstance::l_push_bulltins_type(L, var);
		lua_pushnil(L);
		return 3;
	default:
		luaL_error(L, "Cannot pairs an %s value", var.get_type_name(var.get_type()));
		break;
	}
	return 0;
}

int LuaInstance::meta_bultins__tostring(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Variant* var = luaL_checkvariant(L, 1);

	char buf[4096];
	sprintf(buf, "%s[%s]", var->get_type_name(var->get_type()).utf8().get_data(), (var->operator String()).utf8().get_data());;
	lua_pushstring(L, buf);

	return 1;
}

int LuaInstance::meta_bultins__index(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Variant* var = luaL_checkvariant(L, 1);

	Variant key;
	l_get_variant(L, 2, key);

	bool valid = false;
	Variant value = var->get(key, &valid);
	if(valid) {

		l_push_variant(L, value);
		return 1;	
	}

	if(lua_type(L, 2) == LUA_TSTRING) {

		lua_pushvalue(L, 2);
		lua_pushcclosure(L, l_bultins_caller_wrapper, 1);
		return 1;
	}
	return 0;
}

int LuaInstance::meta_bultins__newindex(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Variant* var = luaL_checkvariant(L, 1);

	Variant key, value;
	l_get_variant(L, 2, key);
	l_get_variant(L, 3, value);

	bool valid = false;
	var->set(key, value, &valid);
	if(!valid)
		luaL_error(L, "Unable to set field: '%s'", ((String) key).utf8().get_data());

	return 0;
}

int LuaInstance::l_push_bulltins_type(lua_State *L, const Variant& var)
{
	LUA_MULTITHREAD_GUARD();

	void *ptr = lua_newuserdata(L, sizeof(Variant*));
	*((Variant **) ptr) = memnew(Variant);
	**((Variant **) ptr) = var;
	luaL_getmetatable(L, "LuaVariant");
	lua_setmetatable(L, -2);

	return 1;
}

void LuaInstance::l_get_variant(lua_State *L, int idx, Variant& var)
{
	LUA_MULTITHREAD_GUARD();

	switch(lua_type(L, idx)) {
	case LUA_TNONE:
	case LUA_TNIL:
	case LUA_TTHREAD:
	case LUA_TLIGHTUSERDATA:
	case LUA_TFUNCTION:
		var = Variant();
		break;

	case LUA_TTABLE:
		break;

	case LUA_TBOOLEAN:
		var = (lua_toboolean(L, idx) != 0);
		break;

	case LUA_TNUMBER:
		if(lua_isinteger(L, idx))
			var = lua_tointeger(L, idx);
		else
			var = lua_tonumber(L, idx);
		break;

	case LUA_TSTRING:
		var = lua_tostring(L, idx);
		break;

	case LUA_TUSERDATA: {

			void *p = lua_touserdata(L, idx);
			if(p != NULL) {

				if(lua_getmetatable(L, idx)) {

					lua_getfield(L, LUA_REGISTRYINDEX, "LuaObject");
					if(lua_rawequal(L, -1, -2)) {

						lua_pop(L, 2);
						size_t id = *((size_t *) p);
						Object *obj = ObjectDB::get_instance(id);
						if(obj->is_type_ptr(Reference::get_type_ptr_static())) {
							Reference *ref = obj->cast_to<Reference>();
							var = Ref<Reference>(ref);
						} else {
							var = obj;
						}
						return;
					}
					lua_pop(L, 1);

					lua_getfield(L, LUA_REGISTRYINDEX, "LuaVariant");
					if(lua_rawequal(L, -1, -2)) {

						lua_pop(L, 2);
						var = **((Variant **) p);
						return;
					}
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
			}
		}
		break;
	}
}

void LuaInstance::l_push_variant(lua_State *L, const Variant& var)
{
	LUA_MULTITHREAD_GUARD();

	switch(var.get_type()) {
	case Variant::NIL:
		lua_pushnil(L);
		break;

	case Variant::BOOL:
		lua_pushboolean(L, ((bool) var) ? 1 : 0);
		break;

	case Variant::INT:
		lua_pushinteger(L, (int) var);
		break;

	case Variant::REAL:
		lua_pushnumber(L, (double) var);
		break;

	case Variant::STRING:
		lua_pushstring(L, ((String) var).utf8().get_data());
		break;

	case Variant::OBJECT: {

			Object *obj = var;
			if(obj == NULL) {
				lua_pushnil(L);
				break;
			}

			ScriptInstance *sci = obj->get_script_instance();
			if(sci == NULL) {
				LuaInstance* instance = memnew( LuaInstance );
				instance->base_ref=false;
				instance->gc_delete=false;
				instance->owner=obj;
				//instance->members.resize(member_indices.size());
				instance->script=Ref<Script>(); //Ref<LuaScript>(obj->get_script_instance());
				obj->set_script_instance(instance);
				if(instance->init(false) != OK) {
					instance->script=Ref<LuaScript>();
					memdelete(instance);
					ERR_FAIL(); //error consrtucting
				}
				sci = instance;
			}
			LuaInstance *inst = dynamic_cast<LuaInstance *>(sci);
			if(inst != NULL && inst->l_get_object_table()) {

				lua_pushstring(L, ".c_instance");
				lua_rawget(L, -2);
				if(!lua_isnil(L, -1)) {

					lua_remove(L, -2);
					break;
				}
				lua_pop(L, 2);

			} else {

				size_t* ptr = (size_t *) lua_newuserdata(L, sizeof(size_t));
				// add refcount if obj is resource
				if(obj->is_type_ptr(Reference::get_type_ptr_static())) {
					Reference *ref = obj->cast_to<Reference>();
					ref->reference();
				}
				*ptr = obj->get_instance_ID();
				luaL_getmetatable(L, "LuaObject");
				lua_setmetatable(L, -2);
			}
		}
		break;

	case Variant::VECTOR2:
	case Variant::RECT2:
	case Variant::VECTOR3:
	case Variant::MATRIX32:
	case Variant::PLANE:
	case Variant::QUAT:
	case Variant::_AABB:
	case Variant::MATRIX3:
	case Variant::TRANSFORM:

	case Variant::COLOR:
	case Variant::IMAGE:
	case Variant::NODE_PATH:
	case Variant::_RID:
	case Variant::INPUT_EVENT:
	case Variant::DICTIONARY:
	case Variant::ARRAY:
	case Variant::RAW_ARRAY:
	case Variant::INT_ARRAY:
	case Variant::REAL_ARRAY:
	case Variant::STRING_ARRAY:
	case Variant::VECTOR2_ARRAY:
	case Variant::VECTOR3_ARRAY:
	case Variant::COLOR_ARRAY:
		l_push_bulltins_type(L, var);
		break;
	}
}

#endif
