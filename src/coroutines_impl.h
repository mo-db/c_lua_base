#pragma once
#include "coroutines_spec.h"
#include "core.h"
#include "app.h"
#include "graphics.h"
#include "render.h"

// typedef void (*ManipFunc)(DynamicObject* dynamic_object, float dt);
// void manip_func1(DynamicObject* dynamic_object, float dt);
// void manip_func2(DynamicObject* dynamic_object, float dt);
// ManipFunc manip_funcs[MANIP_TYPE_COUNT] = { manip_func1, manip_func2 };


void co_init(App* app);
void co_update(App* app, double elapsed_time);

// void co(App* app);
