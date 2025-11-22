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
Vec2 sub_Vec2(const Vec2 v0, const Vec2 v1);
Vec2 mul_Vec2(const Vec2 v, const float s);
Vec2 div_Vec2(const Vec2 v, const float s);
IVec2 add_IVec2(const IVec2 v0, const IVec2 v1);
IVec2 sub_IVec2(const IVec2 v0, const IVec2 v1);
IVec2 mul_IVec2(const IVec2 v, int s);

IVec2 get_IVec2(const Vec2 v);
Vec2 get_Vec2(const IVec2 v);
void norm_Vec2(Vec2 v);
Vec2 get_norm_Vec2(const Vec2 v_in);
Vec2 get_ortho_Vec2(const Vec2 v);
float get_mag_Vec2(const Vec2 v);
float dot_Vec2(const Vec2 v0, const Vec2 v1);
float distance_Vec2(const Vec2 v0, const Vec2 v1);

// --- Rect, Trigon ---
#define RECT_VERT_COUNT 2
#define TRIGON_VERT_COUNT 3
typedef struct Rect {
	union {
		struct {
			Vec2 a, b;
		};
		Vec2 v[2];
	};
} Rect;
Rect new_Rect(const Vec2 a, const Vec2 b);

typedef struct IRect {
	union {
		struct {
			IVec2 a, b;
		};
		IVec2 v[2];
	};
} IRect;
IRect new_IRect(const IVec2 a, const IVec2 b);

typedef struct Trigon {
	union {
		struct {
			Vec2 a, b, c;
		};
		Vec2 v[3];
	};
} Trigon;
Trigon new_Trigon(const Vec2 a, const Vec2 b, const Vec2 c);

typedef struct ITrigon {
	union {
		struct {
			IVec2 a, b, c;
		};
		IVec2 v[3];
	};
} ITrigon;

ITrigon new_ITrigon(const IVec2 a, const IVec2 b, const IVec2 c);


bool rect_contains_point(Vec2 rect_start, Vec2 rect_end, Vec2 p);
int rect_ray_intersect(Vec2 rect_start, Vec2 rect_end, Vec2 line_start,
                        Vec2 line_end, Vec2 *ixn_points);
int rect_line_intersect(Vec2 rect_start, Vec2 rect_end, Vec2 line_start,
                        Vec2 line_end, Vec2 *ixn_points);


typedef struct BaryCoords {
	float alpha;
	float beta;
	float gamma;
} BaryCoords;
double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy);
BaryCoords get_bary_coords(Vec2 a, Vec2 b, Vec2 c, Vec2 p);
