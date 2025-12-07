#pragma once
#include "core.h"

bool lua_check(lua_State *L, int result);
void lua_reload_file(lua_State* L, const char* file_name);
void lua_register_function(lua_State *L, int func(lua_State *),
                           const char *global_name);

bool lua_table_number_at(lua_State *L, const uint32_t index, double* value);
bool lua_table_string_at(lua_State *L, const uint32_t index, char* buff, 
												 size_t buff_size);
bool lua_table_get_number(lua_State *L, const char *key, double* value);
bool lua_table_get_string(lua_State *L, const char *key, char* buff, 
												  size_t buff_size);
