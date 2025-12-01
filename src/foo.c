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

void co_init(App* app) {
	CoState* co = &(app->state.co);
	lua_State* L = app->state.L;
	{
		bool result = SSet_alloc(&co->manips, MAX_MANIPS);
		if (!result) { EXIT(); }
	}
	{
		bool result = SSet_alloc(&co->new_manips, MAX_MANIPS);
		if (!result) { EXIT(); }
	}
	{
		bool result = SSet_alloc(&co->dyn_objects, MAX_DYN_OBJECTS);
		if (!result) { EXIT(); }
	}
	co->player_control_object = UINT32_MAX;

	lua_getglobal(L, "load_level");
	if (lua_isfunction(L, -1)) {
		lua_pushlightuserdata(L, co);
		lua_pushnumber(L, 1);
		if (core_lua_check(L, lua_pcall(L, 2, 1, 0))) {
			printf("success\n");
		} else {
			printf("huh?\n");
		}
	}
}

void co_update(App* app, double elapsed_time) {
	CoState* co = &(app->state.co);
	lua_State* L = app->state.L;

	// --- update manipulators ---
	{
		bool result = update_manips(co, L, elapsed_time);
		if (!result) { EXIT(); }
	}

	// TODO: move somewhere --- player control for object ---
	if (co->player_control_object != UINT32_MAX) {
		DynObject* player_object = SSet_get(&co->dyn_objects, co->player_control_object);
		if (player_object == NULL) { EXIT(); }
		Vec2 vel = {};

		if (get_state(app->state.input.w)) { vel = add_Vec2(vel, (Vec2){0, -0.2}); }
		if (get_state(app->state.input.s)) { vel = add_Vec2(vel, (Vec2){0, 0.2}); }
		if (get_state(app->state.input.a)) { vel = add_Vec2(vel, (Vec2){-0.2, 0}); }
		if (get_state(app->state.input.d)) { vel = add_Vec2(vel, (Vec2){0.2, 0}); }

		player_object->position =
			add_Vec2(player_object->position, mul_Vec2(vel, elapsed_time));
	}

	// --- draw dynamic objects ---
	for (uint32_t i = 0; i < SSET_LEN(co->dyn_objects); i++) {
		DynObject* dyn_object = SSet_at(&co->dyn_objects, i);
		if (dyn_object == NULL) { EXIT(); }
		Vec2 pos = dyn_object->position;
		draw_rect(app->my_renderer, pos, add_Vec2(pos, (Vec2){50,50}), 0xFFFFFFFF);
	}
}
