#pragma once

#include <SDL3/SDL.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/luajit.h>
// #include <tree_sitter/api.h>
// #define PCRE2_CODE_UNIT_WIDTH 8
// #include <pcre2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

// --- essential globals ---
#define PI 3.14159265359
#define EPSILON 1e-6

// --- function-like helper macros ---
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define timed


// --- debug macros ---
#define EXIT()\
	do {\
		fprintf(stderr,"ERROR [%s:%d] %s()\n",\
							 __FILE__, __LINE__, __func__);\
		exit(EXIT_FAILURE);\
	} while (0)

// --- inline functions, no loops or recursion, must be small ---
static inline uint32_t strlen_save(const char* str) {
	if (!str) { EXIT(); }
	uint32_t len = strnlen(str, UINT32_MAX);
	if (len == UINT32_MAX) { EXIT(); }
	return len;
}

static inline bool core_epsilon_equal(double x, double y) {
	return (x < y + EPSILON && x > y - EPSILON);
}

static inline bool time_limit_reached(uint64_t frame_start, double frame_time) {
  uint64_t now = SDL_GetPerformanceCounter();
  double elapsed =
      ((double)(now - frame_start) / SDL_GetPerformanceFrequency()) * 1000;
  if (elapsed >= frame_time * 0.9) {
    return true;
  }
  return false;
}
