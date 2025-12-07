#pragma once
#include "core.h"

bool lua_check(lua_State *L, int result);

bool lua_table_number_at(lua_State *L, const uint32_t index, double* value);
bool lua_table_string_at(lua_State *L, const uint32_t index, char* buff, 
												 size_t buff_size);
bool lua_table_get_number(lua_State *L, const char *key, double* value);
bool lua_table_get_string(lua_State *L, const char *key, char* buff, 
												  size_t buff_size);
