#pragma once

#include <SDL3/SDL.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/luajit.h>
#include <tree_sitter/api.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>

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

#define EXIT_MSG(msg)\
	do {\
		fprintf(stderr,"ERROR [%s:%d] %s()\n->%s\n",\
							 __FILE__, __LINE__, __func__, msg);\
		exit(EXIT_FAILURE);\
	} while (0)

bool core_lua_check(lua_State *L, int result);
bool core_epsilon_equal(double x, double y);




