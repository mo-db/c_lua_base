#include "foo.h"

void foo(App* app, Trigon* trigons) {
	lua_State *L = app->state.L;
	int width = app->my_renderer->pixelbuffer.width;
	int height = app->my_renderer->pixelbuffer.height;

	lua_getglobal(L, "MV");
	if (lua_isfunction(L, -1)) {
		lua_pushnumber(L, N_TRIGONS);
		lua_pushlightuserdata(L, trigons);
		if (core_lua_check(L, lua_pcall(L, 2, 1, 0))) {
			printf("success\n");
		} else {
			printf("huh?\n");
		}
	}

	for (int i = 0; i < N_TRIGONS; i++) {
		Trigon tr = trigons[i];
		draw_trigon(app->my_renderer, tr.a, tr.b, tr.c, (int)(SDL_randf() * (float)0xFFFFFFFF));
	}
}


void bar(App* app) {
	if (became_true(app->state.input.shift)) {
		app->state.L = reload_lua();
	}
	lua_State *L = app->state.L;

	// convert lua table to array
	int size = 0;
	lua_getglobal(L, "productions_size");
	if (lua_isnumber(L, -1)) {
		size = lua_tonumber(L, -1);
		printf("size: %d\n", size);
	} else {
		EXIT();
	}
	char* result[size];

	for (int i = 0; i < size; i++) {
		lua_getglobal(L, "productions");
		if (lua_istable(L, -1)) {
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			if (!lua_isstring(L, -1)) { 
				printf("no string");
				return;
			}
			result[i] = (char *)lua_tostring(L, -1);
			printf("result: %s\n", result[i]);
		} else {
			EXIT();
		}
	}
}

void get_tokens() {
	char symbol[0xF-1];
	char condition[0xFF-1];
	char context[0xFF-1];
	char replacement[0xFFF-1];
}
