#include "graphics.h"

// --- Vec2 ---
Vec2 add_Vec2(const Vec2 v0, const Vec2 v1) {
	return (Vec2){v0.x + v1.x, v0.y + v1.y};
}
Vec2 mul_Vec2(const Vec2 v, const float s) {
	return (Vec2){v.x * s, v.y * s};
}
Vec2 div_Vec2(const Vec2 v, const float s) {
	return (Vec2){v.x / s, v.y / s};
}
IVec2 add_IVec2(const IVec2 v0, const IVec2 v1) {
	return (IVec2){v0.x + v1.x, v0.y + v1.y};
}
IVec2 mul_IVec2(const IVec2 v, int s) {
	return (IVec2){v.x * s, v.y * s};
}


IVec2 get_IVec2(const Vec2 v) {
	return (IVec2){(int)(round(v.x)), (int)(round(v.y))};
}
Vec2 get_Vec2(const IVec2 v) {
	return (Vec2){(float)(v.x), (float)(v.y)};
}
void norm_Vec2(Vec2 v) { 
	float mag = get_mag_Vec2(v); v.x /= mag; v.y /= mag;
}
Vec2 get_norm_Vec2(const Vec2 v_in) { 
	Vec2 v = v_in;
	float mag = get_mag_Vec2(v); v.x /= mag; v.y /= mag;
	return v;
}
Vec2 get_ortho_Vec2(const Vec2 v) { 
	return (Vec2){-v.y, v.x}; 
}
float get_mag_Vec2(const Vec2 v) { 
	return sqrt(v.x * v.x + v.y * v.y);
}
float dot_Vec2(const Vec2 v0, const Vec2 v1) {
	return v0.x * v1.x + v0.y * v1.y;
}
float distance_Vec2(const Vec2 v0, const Vec2 v1) {
	float dx = v1.x - v0.x;
	float dy = v1.y - v0.y;
  return sqrt(dx * dx + dy * dy);
}
