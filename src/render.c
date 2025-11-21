#include "render.h"

Renderer* new_renderer(int width, int height) {
  if (width <= 0 || height <= 0) { return NULL; }
	if ((size_t)width > SIZE_MAX / (size_t)height) { return NULL; }

	Renderer* r = calloc(1, sizeof(Renderer));
	if (!r) { return NULL; }
	r->pixelbuffer.pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
	if (!r->pixelbuffer.pixels) { return NULL; }

	r->viewport.scale = 1.0f;
	r->pixelbuffer.width = width;
	r->pixelbuffer.height = height;
	return r;
}

void destroy_renderer(Renderer* renderer) {
	if (renderer->pixelbuffer.pixels == NULL) { exit(EXIT_FAILURE); }
	free(renderer->pixelbuffer.pixels);
	if (renderer == NULL) { exit(EXIT_FAILURE); }
	free(renderer);
}

void renderer_clear(PixelBuffer* pixel_buffer, uint32_t color) {
	int width = pixel_buffer->width;
	int height = pixel_buffer->height;
  if (width <= 0 || height <= 0) { exit(EXIT_FAILURE); }
	if ((size_t)width > SIZE_MAX / (size_t)height) { exit(EXIT_FAILURE); }

	for (int i = 0; i < width * height; i++) {
		pixel_buffer->pixels[i] = color;
	} 
}

void color_pixels(PixelBuffer* pixel_buffer, IVec2* pixels, int count, uint32_t color) {
  for (size_t i = 0; i < count; i++) {
    int x = pixels[i].x;
    int y = pixels[i].y;
		// TODO: maybe remove check, since i clamp in every draw function
    if (x >= 0 && y >= 0 && x < pixel_buffer->width && y < pixel_buffer->height) {
      pixel_buffer->pixels[x + y * pixel_buffer->width] = color;
    }
  }
}

Vec2 world_to_screen(Viewport* viewport, const Vec2 point_world) {
	Vec2 point_screen =
      mul_Vec2(sub_Vec2(point_world, viewport->xy_offset), viewport->scale);
	return point_screen;
}

Vec2 screen_to_world(Viewport *viewport, const Vec2 point_screen) {
  Vec2 point_world =
      add_Vec2(div_Vec2(point_screen, viewport->scale), viewport->xy_offset);
  return point_world;
}

void draw_rect(Renderer* r, Vec2 p1, Vec2 p2, uint32_t color) {
	p1 = world_to_screen(&r->viewport, p1);
	p2 = world_to_screen(&r->viewport, p2);

	int width = r->pixelbuffer.width;
	int height = r->pixelbuffer.height;

	int x1 = round(MAX(MIN(p1.x, p2.x), 0.f));
	int x2 = round(MIN(MAX(p1.x, p2.x), (float)width));
	int y1 = round(MAX(MIN(p1.y, p2.y), 0.f));
	int y2 = round(MIN(MAX(p1.y, p2.y), (float)height));

	if (x1 > r->pixelbuffer.width || x2 < 0.0f ||
			y1 > r->pixelbuffer.height || y2 < 0.0f ) {
		return;
	}

	int pixel_count = (x2 - x1) * (y2 - y1);
	IVec2 pixels[pixel_count];
	int iter = 0;
	for (int i = y1; i < y2; i++) {
		for (int j = x1; j < x2; j++) {
			pixels[iter++] = (IVec2){j, i};
		}
	}
	color_pixels(&r->pixelbuffer, pixels, pixel_count, color);
}
