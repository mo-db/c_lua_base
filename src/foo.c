#include "foo.h"

void foo(App* app) {
	int width = app->my_renderer->pixelbuffer.width;
	int height = app->my_renderer->pixelbuffer.height;

	lua_State *L = app->state.L;

	Vec2 a = {400, 400};
	Vec2 b = {100, 100};
	Vec2 c = {50, 50};
	Vec2 v[] = {a, b};

	Trigon t = new_Trigon(a, b, add_Vec2(a, b));

	// lua_State* L = luaL_newstate(); // creates a lua VM?
	// luaL_openlibs(L); // adds basic libs to VM

	// if (check_lua(L, luaL_dofile(L, "scripts/test.lua"))){
	// 	lua_getglobal(L, "MV");
	// } else {
	// 	EXIT();
	// }

	if (became_true(app->state.input.shift)) {
		// lua_close (L);
		L = luaL_newstate();
		luaL_openlibs(L);
		app->state.L = L;
		if (check_lua(app->state.L, luaL_dofile(app->state.L, "scripts/test.lua"))){
		} else {
			EXIT();
		}
	}

	lua_getglobal(L, "MV");
	if (lua_isfunction(L, -1)) {
		lua_pushlightuserdata(L, &v);
		lua_pushlightuserdata(L, &t);
		// lua_pushnumber(L, 2);
		if (check_lua(L, lua_pcall(L, 2, 1, 0))) {
			printf("success\n");
			// lua has poped the two variables of the stack
			// result is now on top of the stack
		} else {
			printf("huh?\n");
		}
		// if there was an error, the error value is on the stack
	}

	printf("in c: %f, %f\n", v[0].x, v[0].y);
	printf("in c: %f, %f\n", v[1].x, v[1].y);

	draw_trigon(app->my_renderer, t.a, t.b, t.c, 0xFFFFFFFF);

	// for (int i = 0; i < 500; i++) {
	// 	float rand1 = SDL_randf();
	// 	float rand2 = SDL_randf();
	// 	float rand3 = SDL_randf();
		// draw_thick_line(app->my_renderer, (Vec2){300, 300},
				// (Vec2){rand1*width, rand2*height}, 4*rand3, 0xFF0000FF);
	// }
}
