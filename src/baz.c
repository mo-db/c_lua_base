#include "core.h"
#include "hum_lua.h"
#include "hum_ds.h"

char funcs[5][256];
double dubs[5];

int test_func(lua_State* L) {

	return 1;
}

int main() {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	if (!lua_check(L, luaL_dofile(L, "src/baz.lua"))) { EXIT(); }

	lua_getglobal(L, "bobi");
	if (lua_istable(L, -1)) {
		// if (!lua_access_table(L, "k")) {
		double value = 0;
		char buff[256];
		if (!lua_table_get_string(L, "o", buff, sizeof(buff))) {
			printf("HUH\n");
		} else {
			// printf("value: %f\n", value);
			printf("string: %s\n", buff);
		}
	}

	// --- custom lua interpreter ---
	// char buff[256];
	// // int error = 0;
	// while (fgets(buff, sizeof(buff), stdin) != NULL) {
	// 	// error = luaL_loadbuffer(L, buff, strlen(buff), "line") ||
	// 		// lua_pcall(L, 0, 0, 0);
	// 	lua_check(L, (luaL_loadbuffer(L, buff, strlen(buff), "line") ||
	// 								lua_pcall(L, 0, 0, 0)));
	// }

	lua_close(L);
	return 0;
}
