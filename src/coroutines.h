#pragma once
#include "core.h"
#include "graphics.h"

int wrap_lua_create_level(lua_State* L);
int lua_create_dynamic_object(lua_State* L);
int lua_assign_player_control(lua_State* L);
int lua_move_object(lua_State* L);

typedef struct {
	int id;
	bool dead;
	Vec2 position;
	Vec2 size;
	uint32_t color;
} DynObject;

typedef enum {
	MOVE1,
	MOVE2,
	MANIP_TYPE_COUNT
} ManipType;

typedef struct {
  int id;
	int dyn_object_id;
  ManipType manip_type;

	Vec2 start_pos;
	Vec2 target_pos;
	float run_time;
	float time_passed;
  bool done;
} Manip;

typedef struct {
	int width, height;
} Level;

#define MAX_DYN_OBJECTS 1024
#define MAX_MANIPS 64
typedef struct {
	ArrList(DynObject) dyn_objects;
	int dyn_id_counter;
	int player_control_object;
	ArrList(Manip) manips;
	ArrList(Manip) new_manips;
	int manip_id_counter;
	Level level;
} CoState;


typedef bool (*ManipUpdateFunc)(CoState* co, Manip* m, float dt);
bool update_manip_move1(CoState* co, Manip* m, float dt);
bool update_manip_move2(CoState* co, Manip* m, float dt);
bool update_manips(CoState* co, lua_State* L, float elapsed_time);
DynObject* get_dynamic_object_by_id(CoState* co, int id);

