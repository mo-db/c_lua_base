// foo
#pragma once
#include "core.h"
#include "app.h"
#include "graphics.h"
#include "render.h"
#include "lang_gen.h"

#define N_TRIGONS 200

void foo(App* app, Trigon* trigons);
void configure_generator(App* app, Generator* gen, Interpreter* inter);

void co_init(App* app);
void co_update(App* app, double elapsed_time);

double eval_expression(char* expr);
void get_tokens();



void gen_draw_timed(Interpreter* inter, App* app);
