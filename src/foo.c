#include "foo.h"

void foo(App* app, Trigon* trigons) {

	int width = app->my_renderer->pixelbuffer.width;
	int height = app->my_renderer->pixelbuffer.height;

	lua_State *L = app->state.L;

	Vec2 a = {400, 400};
	Vec2 b = {100, 100};
	Vec2 c = {50, 50};
	Vec2 v[] = {a, b};

	Trigon t = new_Trigon(a, b, add_Vec2(a, b));

	if (became_true(app->state.input.shift)) {
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
		lua_pushnumber(L, N_TRIGONS);
		lua_pushlightuserdata(L, trigons);
		if (check_lua(L, lua_pcall(L, 2, 1, 0))) {
			printf("success\n");
		} else {
			printf("huh?\n");
		}
	}

	printf("in c: %f, %f\n", v[0].x, v[0].y);
	printf("in c: %f, %f\n", v[1].x, v[1].y);

	draw_trigon(app->my_renderer, t.a, t.b, t.c, 0xFFFFFFFF);

	for (int i = 0; i < N_TRIGONS; i++) {
		Trigon tr = trigons[i];
		draw_trigon(app->my_renderer, tr.a, tr.b, tr.c, (int)(SDL_randf() * (float)0xFFFFFFFF));
	}

	// for (int i = 0; i < 500; i++) {
	// 	float rand1 = SDL_randf();
	// 	float rand2 = SDL_randf();
	// 	float rand3 = SDL_randf();
		// draw_thick_line(app->my_renderer, (Vec2){300, 300},
				// (Vec2){rand1*width, rand2*height}, 4*rand3, 0xFF0000FF);
	// }
}
