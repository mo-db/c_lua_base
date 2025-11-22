// render
#pragma once

#include "core.h"
#include "graphics.h"

typedef struct PixelBuffer {
  uint32_t *pixels;
  int width;
  int height;
} PixelBuffer;

typedef struct Viewport {
  Vec2 xy_offset;
	float scale;
} Viewport;

typedef struct Renderer {
	PixelBuffer pixelbuffer;
	Viewport viewport;
} Renderer;

Renderer* new_renderer(const int w, const int h);
void destroy_renderer(Renderer* renderer);
void renderer_clear(PixelBuffer* pixel_buffer, uint32_t color);

struct Rect {};
struct Line {};
struct Trigon {};

// --- base ---
void color_pixels(PixelBuffer* pixel_buffer, IVec2* pixels, int count, uint32_t color);
Vec2 world_to_screen(Viewport* viewport, const Vec2 vertex_world);
Vec2 screen_to_world(Viewport* viewport, const Vec2 point_screen);

// --- drawing functions ---
void draw_rect(Renderer* renderer, Vec2 p0, Vec2 p1, uint32_t color);
void draw_lerp_line(Renderer *renderer, Vec2 p0, Vec2 p1, uint32_t color);
void draw_trigon(Renderer *renderer, Vec2 p0, Vec2 p1, Vec2 p2, uint32_t color);
void draw_lerp_line_trigon(PixelBuffer *pixel_buffer, IVec2 p0, IVec2 p1,
                           IVec2 p2, uint32_t color);

void draw_thick_line(Renderer *renderer, Vec2 start, Vec2 end, float wd,
                     uint32_t color);
