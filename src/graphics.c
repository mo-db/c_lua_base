#include "graphics.h"

// --- Vec2 ---
Vec2 add_Vec2(const Vec2 v0, const Vec2 v1) {
	return (Vec2){v0.x + v1.x, v0.y + v1.y};
}
Vec2 sub_Vec2(const Vec2 v0, const Vec2 v1) {
	return (Vec2){v0.x - v1.x, v0.y - v1.y};
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
IVec2 sub_IVec2(const IVec2 v0, const IVec2 v1) {
	return (IVec2){v0.x - v1.x, v0.y - v1.y};
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



// --- rect ---
bool rect_contains_point(Vec2 rect_start, Vec2 rect_end, Vec2 p) {
  float x_min = MIN(rect_start.x, rect_end.x);
  float x_max = MAX(rect_start.x, rect_end.x);
  float y_min = MIN(rect_start.y, rect_end.y);
  float y_max = MAX(rect_start.y, rect_end.y);
  return p.x > x_min && p.x < x_max && p.y > y_min && p.y < y_max;
}

int rect_ray_intersect(Vec2 rect_start, Vec2 rect_end, Vec2 line_start,
                        Vec2 line_end, Vec2 *ixn_points) {
  float x_min = MIN(rect_start.x, rect_end.x);
  float x_max = MAX(rect_start.x, rect_end.x);
  float y_min = MIN(rect_start.y, rect_end.y);
  float y_max = MAX(rect_start.y, rect_end.y);

  int index = 0;

  float dx = line_end.x - line_start.x;
  float dy = line_end.y - line_start.y;
  float m = 0.0f;
  if (!core_epsilon_equal(dx, 0)) {
    m = dy / dx;
  }
  if (!core_epsilon_equal(m, 0)) {
    float ixn_x = (y_min - line_start.y) / m + line_start.x;
    if (ixn_x > x_min && ixn_x < x_max) {
      ixn_points[index++] = (Vec2){ixn_x, y_min};
    }
    float ixn_w = (y_max - line_start.y) / m + line_start.x;
    if (ixn_w > x_min && ixn_w < x_max) {
      ixn_points[index++] = (Vec2){ixn_w, y_max};
    }
  }
  float ixn_y = (x_min - line_start.x) * m + line_start.y;
  if (ixn_y > y_min && ixn_y < y_max) {
    ixn_points[index++] = (Vec2){x_min, ixn_y};
  }
  float ixn_h = (x_max - line_start.x) * m + line_start.y;
  if (ixn_h > y_min && ixn_h < y_max) {
    ixn_points[index++] = (Vec2){x_max, ixn_h};
  }
	return index;
}
int rect_line_intersect(Vec2 rect_start, Vec2 rect_end, Vec2 line_start,
                        Vec2 line_end, Vec2 *ixn_points) {
  float x_min = MIN(rect_start.x, rect_end.x);
  float x_max = MAX(rect_start.x, rect_end.x);
  float y_min = MIN(rect_start.y, rect_end.y);
  float y_max = MAX(rect_start.y, rect_end.y);

  int index = 0;

  float dx = line_end.x - line_start.x;
  float dy = line_end.y - line_start.y;
  float m = 0.0f;
  if (!core_epsilon_equal(dx, 0)) {
    m = dy / dx;
  }
  if (!core_epsilon_equal(m, 0)) {
    float ixn_x = (y_min - line_start.y) / m + line_start.x;
    if (ixn_x > x_min && ixn_x < x_max &&
        MIN(line_start.y, line_end.y) < y_min &&
        MAX(line_start.y, line_end.y) > y_min) {
      ixn_points[index++] = (Vec2){ixn_x, y_min};
    }
    float ixn_w = (y_max - line_start.y) / m + line_start.x;
    if (ixn_w > x_min && ixn_w < x_max &&
        MIN(line_start.y, line_end.y) < y_max &&
        MAX(line_start.y, line_end.y) > y_max) {
      ixn_points[index++] = (Vec2){ixn_w, y_max};
    }
  }
  float ixn_y = (x_min - line_start.x) * m + line_start.y;
  if (ixn_y > y_min && ixn_y < y_max &&
      MIN(line_start.x, line_end.x) < x_min &&
      MAX(line_start.x, line_end.x) > x_min) {
    ixn_points[index++] = (Vec2){x_min, ixn_y};
  }
  float ixn_h = (x_max - line_start.x) * m + line_start.y;
  if (ixn_h > y_min && ixn_h < y_max &&
      MIN(line_start.x, line_end.x) < x_max &&
      MAX(line_start.x, line_end.x) > x_max) {
    ixn_points[index++] = (Vec2){x_max, ixn_h};
  }
	return index;
}
