#include "coroutines_spec.h"
#include "coroutines_impl.h"

ManipUpdateFunc manip_funcs[MANIP_TYPE_COUNT] = { update_manip_move1,
																									update_manip_move2 };

DynObject* get_dynamic_object_by_id(CoState* co, int id) {
	for (int i = 0; i < ARR_LIST_LEN(co->dyn_objects); i++) {
		if (ArrList_at(&co->dyn_objects, i)->id == id) {
			return ArrList_at(&co->dyn_objects, i);
		}
	}
	return NULL;
}

void lua_create_level(CoState* co, int width, int height) {
	co->level.width = width;
	co->level.height = height;
}

int wrap_lua_create_level(lua_State* L) {
	CoState* co = (CoState*)lua_touserdata(L, 1);
	co->level.width = lua_tointeger(L,2);
	co->level.height = lua_tointeger(L,3);
	printf("level: %d, %d\n", co->level.width, co->level.height);
  lua_pushnumber(L, 8888);
	return 1;
}


int create_dynamic_object(CoState* co, Vec2 position) {
	DynObject dyn = {};
	dyn.id = co->dyn_id_counter++;
	dyn.position = position;
	dyn.color = 0xFFFFFFFF;
	ArrList_push_back(&co->dyn_objects, &dyn);
	return dyn.id;
}

int lua_create_dynamic_object(lua_State* L) {
	CoState* co = (CoState*)lua_touserdata(L, 1);
	Vec2 position = {};
	position.x = lua_tonumber(L, 2);
	position.y = lua_tonumber(L, 3);
	int id = create_dynamic_object(co, position);
	lua_pushnumber(L, id);
	return 1;
}

void assign_player_control(CoState* co, int dyn_id) {
	co->player_control_object = dyn_id;
}

int lua_assign_player_control(lua_State* L) {
	CoState* co = (CoState*)lua_touserdata(L, 1);
	int dyn_id = lua_tonumber(L, 2);
	assign_player_control(co, dyn_id);
	return 0;
}

bool update_manip_move2(CoState* co, Manip* manip, float frame_dt) {
}
bool update_manip_move1(CoState* co, Manip* manip, float frame_dt) {

	// get dynamic object from id
	DynObject* dyn_object = get_dynamic_object_by_id(co, manip->dyn_object_id);

	manip->time_passed += frame_dt;
	if (manip->time_passed >= manip->run_time) {
		dyn_object->position = manip->target_pos;
		manip->done = true;
		return true;
	}

	Vec2 v = sub_Vec2(manip->target_pos, manip->start_pos);
	float lerp_time = manip->time_passed/manip->run_time;
	dyn_object->position = add_Vec2(manip->start_pos,mul_Vec2(v, lerp_time));
	return false;
}

// bad name, this actually does to many things, creates object
void move_object(CoState* co, int dyn_object_id, Vec2 pos, float run_time) {
	Manip new_manip = {};
	DynObject* dyn_object = get_dynamic_object_by_id(co, dyn_object_id);
	new_manip.id = co->manip_id_counter++;
	new_manip.dyn_object_id = dyn_object->id;
	new_manip.start_pos = dyn_object->position;
	new_manip.target_pos = pos;
	new_manip.run_time = run_time;
	ArrList_push_back(&co->new_manips, &new_manip);
	printf("len: %ld\n", ARR_LIST_LEN(co->new_manips));
}

int lua_move_object(lua_State* L) {
	CoState* co = (CoState*)lua_touserdata(L, 1);
	Vec2 pos = {};
	int dyn_id = lua_tonumber(L, 2);
	pos.x = lua_tonumber(L, 3);
	pos.y = lua_tonumber(L, 4);
	float run_time = lua_tonumber(L, 5);
	move_object(co, dyn_id, pos, run_time);
	return 0;
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


	printf("len new manips : %ld\n", ARR_LIST_LEN(co->new_manips));
	printf("len manips     : %ld\n", ARR_LIST_LEN(co->manips));

	for (int i = 0; i < ARR_LIST_LEN(co->new_manips); i++) {
		Manip* new_manip = ArrList_at(&co->new_manips, i);
		ArrList_push_back(&co->manips, new_manip);
	}
	ARR_LIST_LEN(co->new_manips) = 0;

	// --- update manipulators ---
	for (int i = 0; i < ARR_LIST_LEN(co->manips); i++) {
		Manip* manip = ArrList_at(&co->manips, i);
		DynObject* dyn = get_dynamic_object_by_id(co, manip->dyn_object_id);

		bool done = manip_funcs[manip->manip_type](co, manip, elapsed_time);
		if (done) {
			lua_getglobal(L, "issue_next_task");
			if(lua_isfunction(L, -1)) {
				lua_pushlightuserdata(L, co);
				lua_pushnumber(L, dyn->id);
				if (core_lua_check(L, lua_pcall(L, 2, 1, 0))) {
				} else {
					EXIT();
				}
			}
		}
	}

	for (int i = 0; i < ARR_LIST_LEN(co->manips); i++) {
		Manip* manip = ArrList_at(&co->manips, i);
		if (manip->done) {
			ArrList_remove(&co->manips, i);
		}
	}



	// printf("elapsed: %f\n", elapsed_time);

	// printf("control object id: %d\n", co->player_control_object);
	// player control for object
	if (co->player_control_object >= 0) {

		DynObject* player_object = get_dynamic_object_by_id(co, co->player_control_object);
		Vec2 vel = {};
		if (get_state(app->state.input.w)) { vel = add_Vec2(vel, (Vec2){0, -0.2}); }
		if (get_state(app->state.input.s)) { vel = add_Vec2(vel, (Vec2){0, 0.2}); }
		if (get_state(app->state.input.a)) { vel = add_Vec2(vel, (Vec2){-0.2, 0}); }
		if (get_state(app->state.input.d)) { vel = add_Vec2(vel, (Vec2){0.2, 0}); }

		// printf("vel %f, %f\n", vel.x, vel.y);

		player_object->position =
			add_Vec2(player_object->position, mul_Vec2(vel, elapsed_time));
			// add_Vec2(player_object->position, vel);
	}


	// for (int i = 0; i < ARR_LIST_CAP(co->dyn_objects); i++) {
	// 	float rand1 = SDL_randf();
	// 	float rand2 = SDL_randf();
	// 	ArrList_at(&co->dyn_objects, i)->position.x = i*10*rand1;
	// 	ArrList_at(&co->dyn_objects, i)->position.y = i*10*rand2;
	// }

	// draw dynamic objects
	for (int i = 0; i < ARR_LIST_LEN(co->dyn_objects); i++) {
		Vec2 pos = ArrList_at(&co->dyn_objects, i)->position;
		draw_rect(app->my_renderer, pos, add_Vec2(pos, (Vec2){50,50}), 0xFFFFFFFF);
	}
}
