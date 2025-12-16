#include "coroutines.h"

const ManipUpdateFunc manip_funcs[MANIP_TYPE_COUNT] = {update_manip_move1,
                                                       update_manip_move2};

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
	dyn.position = position;
	dyn.color = 0xFFFFFFFF;
	uint32_t dyn_object_id = SSet_push_back(co->dyn_objects, dyn);
	if (dyn_object_id == UINT32_MAX) { EXIT(); }
	return dyn_object_id;
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

bool update_manip_move2(Manip* manip, DynObject* dyn_object, float frame_dt) {
	return false;
}
bool update_manip_move1(Manip* manip, DynObject* dyn_object, float frame_dt) {
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

bool update_manips(CoState* co, lua_State* L, float elapsed_time) {

	// --- add new manips to active manips
	for (uint32_t i = 0; i < SSet_len(co->new_manips); i++) {
		Manip* new_manip = SSet_at(co->new_manips, i);
		if (!new_manip) { EXIT(); }
		uint32_t manip_id = SSet_push_back(co->manips, *new_manip);
		if (manip_id == UINT32_MAX) { EXIT(); }
	}
	SSet_clear(co->new_manips);

	// --- run update functions of active manips ---
	// this runs from backk to front to because manips can be removed
	for (int64_t i = (int64_t)SSet_len(co->manips) - 1; i >= 0; i--) {
		Manip* manip = SSet_at(co->manips, i);
		if (!manip) { EXIT(); }

		DynObject* dyn_object = SSet_get(co->dyn_objects, manip->dyn_object_id);
		if (!dyn_object) { EXIT(); }


		bool done = manip_funcs[manip->manip_type](manip, dyn_object, elapsed_time);
		if (done) {
			// --- continue lua coroutine ---
			lua_getglobal(L, "issue_next_task");
			if(lua_isfunction(L, -1)) {
				lua_pushlightuserdata(L, co);
				lua_pushnumber(L, manip->dyn_object_id);
				if (lua_check(L, lua_pcall(L, 2, 1, 0))) {
				} else {
					EXIT();
				}
			}

			// --- remove manip---
			bool result = SSet_remove(co->manips, SSet_id_at(co->manips, i));
			if (!result) { EXIT(); }
		}
	}
	return true;
}

// bad name, this actually does to many things, creates object
void move_object(CoState* co, int dyn_object_id, Vec2 pos, float run_time) {
	Manip new_manip = {};
	DynObject* dyn_object = SSet_get(co->dyn_objects, dyn_object_id);
	if (!dyn_object) { EXIT(); }
	new_manip.dyn_object_id = dyn_object_id;
	new_manip.start_pos = dyn_object->position;
	new_manip.target_pos = pos;
	new_manip.run_time = run_time;
	new_manip.manip_type = MOVE1;
	new_manip.done = false;
	uint32_t manip_id = SSet_push_back(co->new_manips, new_manip);
	if (manip_id == UINT32_MAX) { EXIT(); }
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
