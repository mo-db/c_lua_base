#include "foo.h"

void foo(App* app, Trigon* trigons) {
	lua_State *L = app->state.L;

	// lua_getglobal(L, "MV");
	// if (lua_isfunction(L, -1)) {
	// 	lua_pushnumber(L, N_TRIGONS);
	// 	lua_pushlightuserdata(L, trigons);
	// 	if (lua_check(L, lua_pcall(L, 2, 1, 0))) {
	// 		printf("success\n");
	// 	} else {
	// 		printf("huh?\n");
	// 	}
	// }

	for (int i = 0; i < N_TRIGONS; i++) {
		Trigon tr = trigons[i];
		draw_trigon(app->my_renderer, tr.a, tr.b, tr.c, (int)(SDL_randf() * (float)0xFFFFFFFF));
		// draw_rect(app->my_renderer, (Vec2){300, 300}, (Vec2){700, 700}, 0xFFFFFFFF);
		// draw_rect_test(app->my_renderer, (Vec2){300, 300}, (Vec2){700, 700}, 0xFFFFFFFF);
		// draw_trigon(app->my_renderer, (Vec2){300, 300}, (Vec2){700, 700}, (Vec2){100, 900}, 0xFF00FFFF);
	}
}

/*
void configure_generator(App* app, Generator* gen, Interpreter* inter) {
	lua_State *L = app->state.L;

	lua_getglobal(L, "move_default");
	if (lua_isnumber(L, -1)) {
		gen->move_default =	(double)lua_tonumber(L, -1);
	}
	lua_getglobal(L, "rotate_default");
	if (lua_isnumber(L, -1)) {
		gen->rotate_default =	(double)lua_tonumber(L, -1);
	}
	printf("move: %f, rotate: %f\n", gen->move_default, gen->rotate_default);

	lua_getglobal(L, "segment_node_count");
	if (lua_isnumber(L, -1)) {
		inter->segment_node_count =	(double)lua_tonumber(L, -1);
	}

	// convert lua table to array
	int size = 0;
	lua_getglobal(L, "productions_size");
	if (lua_isnumber(L, -1)) {
		size = lua_tonumber(L, -1);
		printf("size: %d\n", size);
	} else {
		EXIT();
	}
	// char* result[size];

	for (int i = 0; i < size; i++) {
		lua_getglobal(L, "productions");
		if (lua_istable(L, -1)) {
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			if (!lua_isstring(L, -1)) { 
				printf("no string");
				return;
			}
			Production prod = parse_production_str(gen, (char *)lua_tostring(L, -1));
			SSet_push_back(&gen->productions, &prod);
			// result[i] = (char *)lua_tostring(L, -1);
			// printf("result: %s\n", result[i]);
		} else {
			EXIT();
		}
	}

	// TEST
	for (int i = 0; i < SSET_LEN(gen->productions); i++) {
		printf("prod: %d\n", i);
		Production* prod = SSet_at(&gen->productions, i);
		LSView_trim(&prod->symbol);
		LS_print(prod->symbol);
		LSView_trim(&prod->condition);
		LS_print(prod->condition);
		LSView_trim(&prod->context);
		LS_print(prod->context);
		LSView_trim(&prod->replacement);
		LS_print(prod->replacement);
		printf("\n");
	}
}
*/



// void get_tokens() {
// 	printf("### prod ###\n");
// 	Production prod = parse_production_str("hallo : blub : adsf ! joda");
//
//
//
// 	SSet(double) global_vars;
// 	if (!SSet_alloc(&global_vars, 128)) { EXIT(); }
// 	uint32_t char_to_id_map[128];
// 	Var vars[2] = { (Var){'h', 33}, (Var){'j', 123.123} };
// 	for (int i = 0; i < 2; i++) {
// 		bool result = map_Var_to_SSet(&global_vars, char_to_id_map, vars[i]);
// 		if (!result) { EXIT(); }
// 	}
//
// 	printf("j: %f\n", *SSet_get(&global_vars, char_to_id_map['j']));
// 	printf("h: %f\n", *SSet_get(&global_vars, char_to_id_map['h']));
// }

void co_init(App* app) {
	CoState* co = &(app->state.co);
	lua_State* L = app->state.L;
	co->dyn_objects = SSet_new();
	co->manips = SSet_new();
	co->new_manips = SSet_new();
	co->player_control_object = UINT32_MAX;

	lua_getglobal(L, "load_level");
	if (lua_isfunction(L, -1)) {
		lua_pushlightuserdata(L, co);
		lua_pushnumber(L, 1);
		if (lua_check(L, lua_pcall(L, 2, 1, 0))) {
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
		DynObject* player_object = SSet_get(co->dyn_objects, co->player_control_object);
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
	for (uint32_t i = 0; i < DS_LEN(co->dyn_objects); i++) {
		DynObject* dyn_object = SSet_at(co->dyn_objects, i);
		if (dyn_object == NULL) { EXIT(); }
		Vec2 pos = dyn_object->position;
		draw_rect(app->my_renderer, pos, add_Vec2(pos, (Vec2){50,50}), 0xFFFFFFFF);
	}
}
