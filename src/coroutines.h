#pragma once
#include "core.h"
#include "hum_ds.h"
#include "hum_lua.h"
#include "graphics.h"

int wrap_lua_create_level(lua_State* L);
int lua_create_dynamic_object(lua_State* L);
int lua_assign_player_control(lua_State* L);
int lua_move_object(lua_State* L);

typedef struct {
	bool dead; // where to use?
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
#define MAX_MANIPS 1024
typedef struct {
	SSet(DynObject) dyn_objects;
	uint32_t player_control_object;
	SSet(Manip) manips;
	SSet(Manip) new_manips;
	Level level;
} CoState;


typedef bool (*ManipUpdateFunc)(Manip* manip, DynObject* dyn_object, float frame_dt);
bool update_manip_move1(Manip* manip, DynObject* dyn_object, float frame_dt);
bool update_manip_move2(Manip* manip, DynObject* dyn_object, float frame_dt);
bool update_manips(CoState* co, lua_State* L, float elapsed_time);

