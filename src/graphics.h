// graphics.h

#pragma once

#include "core.h"

// --- Vec2 ---
typedef struct Vec2 {
	float x, y;
} Vec2;
typedef struct IVec2 {
	int x, y;
} IVec2;

Vec2 add_Vec2(const Vec2 v0, const Vec2 v1);
Vec2 mul_Vec2(const Vec2 v, const float s);
Vec2 div_Vec2(const Vec2 v, const float s);
IVec2 add_IVec2(const IVec2 v0, const IVec2 v1);
IVec2 mul_IVec2(const IVec2 v, int s);

IVec2 get_IVec2(const Vec2 v);
Vec2 get_Vec2(const IVec2 v);
void norm_Vec2(Vec2 v);
Vec2 get_norm_Vec2(const Vec2 v_in);
Vec2 get_ortho_Vec2(const Vec2 v);
float get_mag_Vec2(const Vec2 v);
float dot_Vec2(const Vec2 v0, const Vec2 v1);
float distance_Vec2(const Vec2 v0, const Vec2 v1);
