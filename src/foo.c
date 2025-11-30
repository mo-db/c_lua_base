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



void sparse_test(App* app) {
	SSet(Blub) new_sset = {};	
	if (!SSet_alloc(&new_sset, 32)) { EXIT(); }
	Blub b1 = { 5.0, 12.5 };
	Blub b2 = { 99.0, 30 };
	Blub b3 = { -700.25, 30.9 };

	uint32_t b1_id = SSet_push_back(&new_sset, &b1);
	uint32_t b2_id = SSet_push_back(&new_sset, &b2);
	uint32_t b3_id = SSet_push_back(&new_sset, &b3);

	for (int i = 0; i < S_SET_COUNT(new_sset); i++) {
		Blub* result = SSet_at(&new_sset, i);
		if (result) {
			printf("result[%d]: %f, %f\n", i, SSet_get(&new_sset, i)->x, SSet_get(&new_sset, i)->y);
		} else {
			EXIT();
		}
	}

	if (!SSet_remove(&new_sset, b1_id)) { EXIT(); }


	for (int i = 0; i < S_SET_COUNT(new_sset); i++) {
		Blub* result = SSet_at(&new_sset, i);
		if (result) {
			printf("result[%d]: %f, %f\n", i, SSet_get(&new_sset, i)->x, SSet_get(&new_sset, i)->y);
		} else {
			EXIT();
		}
	}


	//
	// for (int i = 0; i < S_SET_CAP(new_sset); i++) {
	// 	SSet_get(new_sset, i);
	// }

}


void co_init(App* app) {
	CoState* co = &(app->state.co);
	lua_State* L = app->state.L;
	ArrList_alloc(&co->manips, MAX_MANIPS);
	ArrList_alloc(&co->new_manips, MAX_MANIPS);
	ArrList_alloc(&co->dyn_objects, MAX_DYN_OBJECTS);
	co->player_control_object = -1;


	printf("cap: %ld\n", ARR_LIST_CAP(co->dyn_objects));

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
	if (co->player_control_object >= 0) {
		DynObject* player_object = get_dynamic_object_by_id(co, co->player_control_object);
		Vec2 vel = {};

		if (get_state(app->state.input.w)) { vel = add_Vec2(vel, (Vec2){0, -0.2}); }
		if (get_state(app->state.input.s)) { vel = add_Vec2(vel, (Vec2){0, 0.2}); }
		if (get_state(app->state.input.a)) { vel = add_Vec2(vel, (Vec2){-0.2, 0}); }
		if (get_state(app->state.input.d)) { vel = add_Vec2(vel, (Vec2){0.2, 0}); }

		player_object->position =
			add_Vec2(player_object->position, mul_Vec2(vel, elapsed_time));
	}

	// --- draw dynamic objects ---
	for (int i = 0; i < ARR_LIST_LEN(co->dyn_objects); i++) {
		Vec2 pos = ArrList_at(&co->dyn_objects, i)->position;
		draw_rect(app->my_renderer, pos, add_Vec2(pos, (Vec2){50,50}), 0xFFFFFFFF);
	}
}
