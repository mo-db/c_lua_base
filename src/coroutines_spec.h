#pragma once
#include "core.h"
#include "graphics.h"

typedef struct {
	Vec2 position;
	int id;
} DynamicObject;

typedef struct {
	int id, val;
} Test;

typedef enum {
	MOVE1,
	MOVE2,
	MANIP_TYPE_COUNT
} ManipType;

typedef struct {
  ManipType manip_type;
  int object_id;
  bool done;
} Manipulator;

typedef struct {
	ArrList(DynamicObject)* dynamic_objects;
	ArrList(Manipulator)* manips;
} CoState;
