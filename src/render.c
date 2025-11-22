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

// floating point line algorithm
void draw_lerp_line(Renderer *r, Vec2 p0, Vec2 p1, uint32_t color) {
	IVec2 v0 = get_IVec2(world_to_screen(&r->viewport, p0));
	IVec2 v1 = get_IVec2(world_to_screen(&r->viewport, p1));

	int dx = abs(v1.x - v0.x);
	int dy = abs(v1.y - v0.y);
	int length = MAX(dx, dy);

	IVec2 pixels[length];

	// normal displacement vector for Chebyshev distance
  float x_displace = (float)dx / length;
  float y_displace = (float)dy / length;

	for (int i = 0; i < length; i++) {
		// float t = static_cast<float>(i) / static_cast<float>(length);

		// parametric line can be written in two forms:
		// l(t) = start_point + t * v(start_point, end_point)
		// float x = t * static_cast<float>(delta_x) + static_cast<float>(start.x);
		// float y = t * static_cast<float>(delta_y) + static_cast<float>(start.y);

		// l(t) = (1 - t) * start_point + t * end_point
		// float x = (1.0f - t) * static_cast<float>(start.x) + t * static_cast<float>(end.x);
		// float y = (1.0f - t) * static_cast<float>(start.y) + t * static_cast<float>(end.y);

		// use normal vector to avoid division every step
		pixels[i] = (IVec2){(int)round(i * x_displace + v0.x),
												(int)round(i * y_displace + v0.y)};
	}
	color_pixels(&r->pixelbuffer, pixels, length, color);
}

void draw_lerp_line_trigon(PixelBuffer *pixelbuffer, IVec2 v0, IVec2 v1,
                           IVec2 v2, uint32_t color) {
  bool switched_xy = false;
  if (v0.x == v1.x || v0.x == v2.x || v1.x == v2.x) {
		switched_xy = true;
		int temp = v0.x;
		v0.x = v0.y;
		v0.y = temp;
		temp = v1.x;
		v1.x = v1.y;
		v1.y = temp;
		temp = v2.x;
		v2.x = v2.y;
		v2.y = temp;
  } else if (v0.y == v1.y || v0.y == v2.y || v1.y == v2.y) {
		// do nothing
  } else {
		exit(EXIT_FAILURE);

  }
	// fmt::print("v0: {}, {}\nv1: {}, {}\nv2 {}, {}\n", v0.x, v0.y, v1.x, v1.y,
	// 		v2.x, v2.y);
	// fmt::print("switched: {}\n", switched_xy);

	// ---- sort the vertices ----
	IVec2 v_start = {};
	IVec2 v_left = {};
	IVec2 v_right = {};
	if (v0.y == v1.y) { 
		v_start = v2; 
		if (v0.x < v1.x) {
			v_left = v0;
			v_right = v1;
		}
		else {
			v_left = v1;
			v_right = v0;
		}
	}
	else if (v0.y == v2.y) { 
		v_start = v1;
		if (v0.x < v2.x) {
			v_left = v0;
			v_right = v2;
		}
		else {
			v_left = v2;
			v_right = v0;
		}
	}
	else if (v1.y == v2.y) {
		v_start = v0;
		if (v1.x < v2.x) {
			v_left = v1;
			v_right = v2;
		}
		else {
			v_left = v2;
			v_right = v1;
		}
	}

	int dx_left = abs(v_left.x - v_start.x);
	int dy_left = abs(v_left.y - v_start.y);
	int sx_left = (v_start.x < v_left.x) ? 1 : -1;
	int sy_left = (v_start.y < v_left.y) ? 1 : -1;
	int length_left = MAX(dx_left, dy_left);

	int dx_right = abs(v_right.x - v_start.x);
	int dy_right = abs(v_right.y - v_start.y);
	int sx_right = (v_start.x < v_right.x) ? 1 : -1;
	int sy_right = (v_start.y < v_right.y) ? 1 : -1;
	int length_right = MAX(dx_right, dy_right);

	if (length_right == 0 || length_left == 0) { return; }

	// if (dx_left > pixelbuffer->width ||
	// 		dx_right > pixelbuffer->width ||
	// 		dy_left > pixelbuffer->width ||
	// 		dy_right > pixelbuffer->width) {
		printf("dx_left: %d, dx_right: %d, dy_left: %d, dy_right %d\n",
				dx_left, dx_right, dy_left, dy_right);
		// return;
	// }


	// TODO: reserver less space?
	int pixel_count = 0;
	int max_pixels = pixelbuffer->width * pixelbuffer->height;
	IVec2 *pixels = malloc(sizeof(IVec2) * max_pixels);
	if (!pixels) return; // OOM guard

	
	// int max_pixels = pixelbuffer->width * pixelbuffer->height;
	// IVec2 pixels[max_pixels];
	// pixels.reserve(dy_left * (dx_left + dx_right));

	// normal displacement vector for Chebyshev distance
  float x_displace_left = ((float)dx_left / length_left) * sx_left;
  float y_displace_left = ((float)dy_left / length_left) * sy_left;

  float x_displace_right = ((float)dx_right / length_right) * sx_right;
  float y_displace_right = ((float)dy_right / length_right) * sy_right;

	IVec2 left = v_start;
	IVec2 right = v_start;
	int last_y = 0;

	int index_left = 0;
	int index_right = 0;

	for (;;) {
		// left
		last_y = left.y;
		while (left.y == last_y) {
			left.x = round(index_left * x_displace_left + v_start.x);
			left.y = round(index_left * y_displace_left + v_start.y);
			if (index_left >= length_left) { break; }
			index_left++;
		}

		// right
		last_y = right.y;
		while (right.y == last_y) {
			right.x = round(index_right * x_displace_right + v_start.x);
			right.y = round(index_right * y_displace_right + v_start.y);
			if (index_right >= length_right) { break; }
			index_right++;
		}

		// fill
		for (int i = left.x; i <= right.x; i++) {
			if (pixel_count < max_pixels) {
				if (switched_xy) {
					pixels[pixel_count++] = (IVec2){left.y, i};
				} else {
					pixels[pixel_count++] = (IVec2){i, left.y};
				}
			}
		}

		if (index_left == length_left || index_right == length_right) { break; }
	}
	free(pixels);
	color_pixels(pixelbuffer, pixels, pixel_count, color);
}


typedef struct IndexAngle {
	int index;
	float angle;
} IndexAngle;

int compare_angles(const void* in_0, const void* in_1){ 
	IndexAngle* ia0 = (IndexAngle*)in_0;
	IndexAngle* ia1 = (IndexAngle*)in_1;
	if (ia0->angle < ia1->angle) {
		return -1;
	} else if (ia0->angle > ia1->angle) {
		return 1;
	} else {
		return 0;
	}
}

void draw_trigon(Renderer *r, Vec2 p0, Vec2 p1, Vec2 p2, uint32_t color) {

	p0 = world_to_screen(&r->viewport, p0);
	p1 = world_to_screen(&r->viewport, p1);
	p2 = world_to_screen(&r->viewport, p2);

	// define screen borders
	Vec2 rect_start = {200.0f, 200.0f};
	Vec2 rect_end = {(float)r->pixelbuffer.width - 200.0f,
								(float)r->pixelbuffer.height - 200.0f};

	// all relevant points to build trigons
	Vec2 fpoints[4];

	// calculate ixn_points with screen borders
	Vec2 ixn_points0[2];
	Vec2 ixn_points1[2];
	Vec2 ixn_points2[2];
	int ixn_points0_count =
			rect_line_intersect(rect_start, rect_end, p0, p1, ixn_points0);
	int ixn_points1_count =
			rect_line_intersect(rect_start, rect_end, p0, p2, ixn_points1);
	int ixn_points2_count =
			rect_line_intersect(rect_start, rect_end, p1, p2, ixn_points2);

	int fpoint_count = 0;

  // add ixn_points to fpoints
	for (size_t i = 0; i < ixn_points0_count; i++) {
		fpoints[fpoint_count++] = ixn_points0[i];
	}
	for (size_t i = 0; i < ixn_points1_count; i++) {
		fpoints[fpoint_count++] = ixn_points1[i];
	}
	for (size_t i = 0; i < ixn_points2_count; i++) {
		fpoints[fpoint_count++] = ixn_points2[i];
	}

	// add points inside rect borders to fpoints
	if (rect_contains_point(rect_start, rect_end, p0)) {
		fpoints[fpoint_count++] = p0;
	}
	if (rect_contains_point(rect_start, rect_end, p1)) {
		fpoints[fpoint_count++] = p1;
	}
	if (rect_contains_point(rect_start, rect_end, p2)) {
		fpoints[fpoint_count++] = p2;
	}

	if (fpoint_count < 3) { return; }

	// round all points to nerest int
	int point_count = fpoint_count;
	IVec2 points[fpoint_count];
	for (size_t i = 0; i < fpoint_count; i++) {
		// points.push_back(world_to_screen(viewport, fpoints[i]).get_IVec2());
		points[i] = get_IVec2(fpoints[i]);
	}

	// fmt::print("points.size(): {}\n", points.size());
	typedef struct ITrigon {
		IVec2 p0;
		IVec2 p1;
		IVec2 p2;
	} ITrigon;
	ITrigon trigons[2]; // max possible trigon amount

	int trigon_count = 0;

	// ---- split quad into trigons ----
	if (point_count == 4) {
		Vec2 centeroid = {};
		for (int i = 0; i < 4; i++) {
			centeroid.x += points[i].x;
			centeroid.y += points[i].y;
		}
		centeroid = mul_Vec2(centeroid, 0.25);

		IndexAngle index_angles[4];
		for (int i = 0; i < 4; i++) {
			Vec2 v = sub_Vec2(get_Vec2(points[i]), centeroid);
			index_angles[i] = (IndexAngle){i, atan2(v.y, v.x)};
		}

		qsort(index_angles, 4, sizeof(IndexAngle), compare_angles);


		trigons[trigon_count++] = (ITrigon){points[index_angles[0].index],
			points[index_angles[2].index], points[index_angles[1].index]};
		trigons[trigon_count++] = (ITrigon){points[index_angles[0].index],
			points[index_angles[2].index], points[index_angles[3].index]};
	} else {
		trigons[trigon_count++] = (ITrigon){points[0], points[1], points[2]};
	}

	// ---- draw all trigons ----
	for (int i = 0; i < trigon_count; i++) {
		ITrigon trigon = trigons[i];

		// early continue if trigon has no area
		if ((trigon.p0.x == trigon.p1.x && trigon.p0.y == trigon.p1.y) ||
				(trigon.p0.x == trigon.p2.x && trigon.p0.y == trigon.p2.y) ||
				(trigon.p1.x == trigon.p2.x && trigon.p1.y == trigon.p2.y)) {
			continue;
		}

		// if allready flat, draw right away
		if (trigon.p0.x == trigon.p1.x ||
				trigon.p0.x == trigon.p2.x ||
				trigon.p1.x == trigon.p2.x ||
				trigon.p0.y == trigon.p1.y ||
				trigon.p0.y == trigon.p2.y ||
				trigon.p1.y == trigon.p2.y) {
			draw_lerp_line_trigon(&r->pixelbuffer, trigon.p0,
														trigon.p1, trigon.p2, color);
		} else {
		// sort for smallest y coordinate
			do {
				IVec2 temp = trigon.p0;
				if (temp.y > trigon.p1.y) {
					trigon.p0 = trigon.p1;
					trigon.p1 = temp;
				}

				if (trigon.p2.y < trigon.p1.y) {
					temp = trigon.p1;
					trigon.p1 = trigon.p2;
					trigon.p2 = temp;
				}
			} while (trigon.p0.y > trigon.p1.y || trigon.p1.y > trigon.p2.y);

			// split horizontally, draw both splits
			float k = (float)(trigon.p1.y - trigon.p0.y) /
								(trigon.p2.y - trigon.p0.y);
			IVec2 p3 = {(int)(round(trigon.p0.x + k * 
								(trigon.p2.x - trigon.p0.x))), trigon.p1.y};
			draw_lerp_line_trigon(&r->pixelbuffer, trigon.p0,
														trigon.p1, p3, color);
			draw_lerp_line_trigon(&r->pixelbuffer, trigon.p2,
														trigon.p1, p3, color);
		}
	}
}

void draw_thick_line(Renderer *r, Vec2 start, Vec2 end, float wd,
                     uint32_t color) {
	Vec2 line = sub_Vec2(end, start);

	Vec2 a = get_ortho_Vec2(line);
	norm_Vec2(a);

	Vec2 p0 = add_Vec2(start, mul_Vec2(a, (wd/2.0f)));
	Vec2 p1 = sub_Vec2(p0, mul_Vec2(a, wd));
	Vec2 p2 = add_Vec2(p1, line);
	Vec2 p3 = add_Vec2(p0, line);

	draw_trigon(r, p0, p1,p3, color);
	draw_trigon(r, p1, p2,p3, color);
}
