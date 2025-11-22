#pragma once

#include <SDL3/SDL.h>
#include <lua.h>
#include <stdio.h>
#include <stdlib.h>
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

bool core_epsilon_equal(double x, double y);
