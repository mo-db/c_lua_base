// foo
#pragma once
#include "core.h"
#include "app.h"
#include "graphics.h"
#include "render.h"

#define N_TRIGONS 200

void foo(App* app, Trigon* trigons);
void bar(App* app);

void co_init(App* app);
void co_update(App* app, double elapsed_time);

double eval_expression(char* expr);
