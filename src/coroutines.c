#include "coroutines_spec.h"
#include "coroutines_impl.h"

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
	return true;
}


void co_init(App* app) {
	CoState* co = &(app->state.co);
	lua_State* L = app->state.L;
	ArrList_alloc(&co->manips, MAX_MANIPS);
	ArrList_alloc(&co->dyn_objects, MAX_DYN_OBJECTS);
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

void co_update(App* app) {
	CoState* co = &(app->state.co);
	lua_State* L = app->state.L;



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
