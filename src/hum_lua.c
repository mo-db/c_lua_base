#include "hum_lua.h"

bool lua_check(lua_State *L, int result) {
	if (result != LUA_OK) {
		const char* error_msg = lua_tostring(L, -1);
		printf("LUA_ERROR %s\n", error_msg);
		return false;
	}
	return true;
}

void lua_reload_file(lua_State* L, const char* file_name) {
	if (!lua_check(L, luaL_dofile(L, file_name))) {
		EXIT();
	}
}

void lua_register_function(lua_State *L, int func(lua_State *),
                           const char *global_name) {
  lua_pushcfunction(L, func);
	lua_setglobal(L, global_name);
}

bool lua_table_empty(lua_State *L, int idx) {
    lua_pushnil(L);
    if (!lua_next(L, idx - 1)) {
        // no elements
        return true;
    }
    // table has at least one key-value pair
    lua_pop(L, 2); // pop value and key
    return false;
}

bool lua_table_number_at(lua_State *L, const uint32_t index, double* value) {
	/* push the key onto the stack */
	lua_pushnumber(L, index);
	/* get the field i.e. table[key], if key invalid, lua will push nil */
	lua_gettable(L, -2);
	
	if (!lua_isnumber(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	*value = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return true;
}

bool lua_table_string_at(lua_State *L, const uint32_t index, Str *str) {
	lua_pushnumber(L, index);
	lua_gettable(L, -2);
	
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	Str_put_cstr(str, lua_tolstring(L, -1, NULL));
	lua_pop(L, 1);
	return true;
}

bool lua_table_get_number(lua_State *L, const char *key, double* value) {
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	
	if (!lua_isnumber(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	*value = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return true;
}

bool lua_table_get_string(lua_State *L, const char *key, Str *str) {
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	Str_put_cstr(str, lua_tolstring(L, -1, NULL));
	lua_pop(L, 1);
	return true;
}
