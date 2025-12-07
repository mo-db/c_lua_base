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

bool lua_table_string_at(lua_State *L, const uint32_t index, char* buff, 
												 size_t buff_size) {
	lua_pushnumber(L, index);
	lua_gettable(L, -2);
	
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	size_t len = 0;
	const char* lua_string = lua_tolstring(L, -1, &len);
	if (len > buff_size) {
		lua_pop(L, 1);
		return false;
	}

	memcpy(buff, lua_string, len);
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

bool lua_table_get_string(lua_State *L, const char *key, char* buff, 
												  size_t buff_size) {
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	size_t len = 0;
	const char* lua_string = lua_tolstring(L, -1, &len);
	if (len > buff_size) {
		lua_pop(L, 1);
		return false;
	}

	memcpy(buff, lua_string, len);
	lua_pop(L, 1);
	return true;
}



bool lua_access_table(lua_State *L, const char *key) {
	lua_pushstring(L, key); /* push the key onto the stack */
	lua_gettable(L, -2); /* get the field i.e. table[key] */
	
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	size_t len = 0;
	printf("table field: %s\n", lua_tolstring(L, -1, &len));
	printf("table len: %lu \n", len);

	lua_pop(L, 1);
	return true;
}

bool lua_table_get_number_value(lua_State *L, const char *key, double value) {
	/* push the key onto the stack */
	lua_pushstring(L, key); 
	/* get the field i.e. table[key], if key invalid, lua will push nil */
	lua_gettable(L, -2); 
	
	if (lua_isnumber(L, -1)) {
		size_t len = 0;
		printf("table field: %s\n", lua_tolstring(L, -1, &len));
		printf("table len: %lu \n", len);
		lua_pop(L, 1);
		return false;
	}
	lua_pop(L, 1);
	return true;
}
