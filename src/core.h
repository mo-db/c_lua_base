#pragma once

#include <SDL3/SDL.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/luajit.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define PI 3.14159265359
#define EPSILON 1e-6

#define EXIT()\
	do {\
		fprintf(stderr,"ERROR [%s:%d] %s()\n",\
							 __FILE__, __LINE__, __func__);\
		exit(EXIT_FAILURE);\
	} while (0)


bool check_lua(lua_State *L, int result);
bool core_epsilon_equal(double x, double y);
