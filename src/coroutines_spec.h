#pragma once
#include "core.h"
#include "graphics.h"

int wrap_lua_create_level(lua_State* L);
int lua_create_dynamic_object(lua_State* L);

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
  ManipType manip_type;
  int object_id;
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
	ArrList(Manip) manips;
	int manips_id_counter;
	Level level;
} CoState;
