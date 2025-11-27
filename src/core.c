#include "core.h"

bool core_lua_check(lua_State *L, int result) {
	if (result != LUA_OK) {
		const char* error_msg = lua_tostring(L, -1);
		printf("LUA_ERROR %s\n", error_msg);
		return false;
	}
	return true;
}

bool core_epsilon_equal(double x, double y) {
	return (x < y + EPSILON && x > y - EPSILON);
}
