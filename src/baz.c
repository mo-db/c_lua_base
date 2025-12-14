#include "core.h"
#include "hum_lua.h"
#include "hum_ds.h"
#include "graphics.h"

char funcs[5][256];
double dubs[5];

int test_func(lua_State* L) {

	return 1;
}

SSET2_DEFINE(SSetVec2, Vec2);
SSET2_DEFINE(SSetInt, int);
SSET2_DEFINE(SSetDub, double);

int main() {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	if (!lua_check(L, luaL_dofile(L, "src/baz.lua"))) { EXIT(); }

	// SSetVec2 *sset_vec2 = SSet2_new();
	SSetInt *sset_int = SSet2_new();
	SSetDub *sset_dub = SSet2_new();

	if (!SSet2_emplace_back(sset_dub, 'i', ((double){1.1}))) { EXIT(); }
	if (!SSet2_emplace_back(sset_dub, 'j', ((double){22.22}))) { EXIT(); }

	for (int i = 0; i < sset_dub->internal.len; i++) {
		printf("dub: %f\n", *SSet2_at(sset_dub, i));
	}

	if (!SSet2_remove(sset_dub, 'j')) { EXIT(); }

	if (!SSet2_emplace_back(sset_dub, 'k', ((double){1234.4321}))) { EXIT(); }
	if (!SSet2_emplace_back(sset_dub, 'j', ((double){888.999}))) { EXIT(); }

	for (int i = 0; i < sset_dub->internal.len; i++) {
		printf("dub: %f\n", *SSet2_at(sset_dub, i));
	}


	// uint32_t id0 = SSet2_push_back(sset_vec2, ((Vec2){3.3, 4.4}));
	// uint32_t id1 = SSet2_push_back(sset_vec2, ((Vec2){3, 4}));
	bool did = SSet2_emplace_back(sset_int, 'k', ((int){88}));
	uint32_t id3 = SSet2_push_back(sset_int, ((int){99}));

	if (did) {
		printf("k id: %d\n", *SSet2_get(sset_int, 'k'));
	}

	for (int i = 0; i < 300; i++) {
		SSet2_push_back(sset_int, ((int){i*10000}));
	}


	Str *st = Str_new();

	if (sset_int) {
		printf("huh\n");
	}

	if (!SSet2_get(sset_int, 400)){
		printf("didnt get\n");
	} else {
		printf("did get\n");
	}

	if (!SSet2_get(sset_int, 200)){
		printf("didnt get\n");
	} else {
		printf("did get\n");
	}


	SSet2_remove(sset_int, id3);
	if (SSet2_get(sset_int, id3)){
		printf("id3: %d\n", *SSet2_get(sset_int, id3));
	}
	SSet2_free(&sset_int);

	

	// for (int i = 0; i < sset_int->internal.len; i++) {
	// 	printf("int: %d\n", *SSet2_at(sset_int, i));
	// }

	return 0;



	// Str str1 = {};
	Str *str2 = Str_new();

	Str_put_cstr(str2, "Panther");
	StrView view2 = Str_get_view(str2);
	StrView_print(&view2);
	for (int i = 0; i < 50; i++) {
		Str_put_cstr(str2, "Panther");
	}
	printf("cap: %d, len: %d\n", str2->cap, str2->len);
	StrView_print(&view2);
	Str_print(str2);

	// Str_put_cstr(&str1, "Backoven");
	// Str_print(&str1);
	// Str_putc(&str1, 'A');
	// Str_print(&str1);
	// printf("HUH: %s\n", str1.data);

	Str_free(str2);
	







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
