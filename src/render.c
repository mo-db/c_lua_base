#include "render.h"

Renderer* new_renderer(int width, int height) {
  if (width <= 0 || height <= 0) { return NULL; }
	uint64_t n_pixels = (uint64_t)width * (uint64_t)height;
	if (n_pixels > INT_MAX) { EXIT(); }

	Renderer* r = calloc(1, sizeof(Renderer));
	if (!r) { return NULL; }
	r->pixelbuffer.pixels = (uint32_t*)malloc(n_pixels * sizeof(uint32_t));
	if (!r->pixelbuffer.pixels) { return NULL; }

	r->viewport.scale = 1.0f;
	r->pixelbuffer.width = width;
	r->pixelbuffer.height = height;
	return r;
}

void destroy_renderer(Renderer* renderer) {
	if (renderer->pixelbuffer.pixels == NULL) { EXIT(); }
	free(renderer->pixelbuffer.pixels);
	if (renderer == NULL) { EXIT(); }
	free(renderer);
}

void renderer_clear(PixelBuffer* pixel_buffer, uint32_t color) {
	int width = pixel_buffer->width;
	int height = pixel_buffer->height;

  if (width <= 0 || height <= 0) { EXIT(); }
	uint64_t n_pixels = (uint64_t)width * (uint64_t)height;
	if (n_pixels > INT_MAX) { EXIT(); }

	for (int i = 0; i < n_pixels; i++) {
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

void set_pixel(PixelBuffer* fb, int x, int y, uint32_t color) {
	if (x >= 0 && y >= 0 && x < fb->width && y < fb->height) {
		fb->pixels[x + y * fb->width] = color;
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

void draw_rect(Renderer* r, Vec2 start, Vec2 end, uint32_t color) {
	IVec2 p0 = get_IVec2(world_to_screen(&r->viewport, start));
	IVec2 p1 = get_IVec2(world_to_screen(&r->viewport, end));

	int width = r->pixelbuffer.width;
	int height = r->pixelbuffer.height;

	int x_min_raw = round(MIN(p0.x, p1.x));
	int x_max_raw = round(MAX(p0.x, p1.x));
	int y_min_raw = round(MIN(p0.y, p1.y));
	int y_max_raw = round(MAX(p0.y, p1.y));

	int x_min = MIN(MAX(x_min_raw, 0.0), width);
	int x_max = MIN(MAX(x_max_raw, 0.0), width);
	int y_min = MIN(MAX(y_min_raw, 0.0), height);
	int y_max = MIN(MAX(y_max_raw, 0.0), height);

	int dx = x_max - x_min;
	int dy = y_max - y_min;

	if (dx == 0 || dy == 0) {
		return;
	}

	IVec2 pixels[width];
	int pixel_count;
	for (int i = y_min; i < y_max; i++) {
		pixel_count = 0;
		for (int j = x_min; j < x_max; j++) {
			pixels[pixel_count++] = (IVec2){j, i};
		}
		color_pixels(&r->pixelbuffer, pixels, pixel_count, color);
	}
}


void draw_rect_test(Renderer* fb, Vec2 p0, Vec2 p1, uint32_t color) {
	// IVec2 p0 = get_IVec2(world_to_screen(&r->viewport, start));
	// IVec2 p1 = get_IVec2(world_to_screen(&r->viewport, end));

	int width = fb->pixelbuffer.width;
	int height = fb->pixelbuffer.height;
	printf("width: %d, height: %d\n", width, height);

	int x_min_raw = round(MIN(p0.x, p1.x));
	int x_max_raw = round(MAX(p0.x, p1.x));
	int y_min_raw = round(MIN(p0.y, p1.y));
	int y_max_raw = round(MAX(p0.y, p1.y));

	int x_min = MIN(MAX(x_min_raw, 0.0), width);
	int x_max = MIN(MAX(x_max_raw, 0.0), width);
	int y_min = MIN(MAX(y_min_raw, 0.0), height);
	int y_max = MIN(MAX(y_max_raw, 0.0), height);

	int dx = x_max - x_min;
	int dy = y_max - y_min;

	if (dx == 0 || dy == 0) {
		return;
	}

	// Vec2 pixels[width];
	// int pixel_count;
	for (int i = y_min; i < y_max; i++) {
		// pixel_count = 0;
		for (int j = x_min; j < x_max; j++) {
			// pixels[pixel_count++] = (Vec2){j, i};
			set_pixel(&fb->pixelbuffer, j, i, color);
		}
	}
}


// floating point line algorithm
void draw_lerp_line(Renderer *r, Vec2 p0, Vec2 p1, uint32_t color) {
	IVec2 v0 = get_IVec2(world_to_screen(&r->viewport, p0));
	IVec2 v1 = get_IVec2(world_to_screen(&r->viewport, p1));

	int dx = v1.x - v0.x;
	int dy = v1.y - v0.y;
	int length = MAX(abs(dx), abs(dy));

	IVec2 pixels[length];

	// normal displacement vector for Chebyshev distance
  float x_displace = (float)dx / length;
  float y_displace = (float)dy / length;

	for (int i = 0; i < length; i++) {
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
		EXIT();
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
		// printf("dx_left: %d, dx_right: %d, dy_left: %d, dy_right %d\n",
				// dx_left, dx_right, dy_left, dy_right);
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
	color_pixels(pixelbuffer, pixels, pixel_count, color);
	free(pixels);
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

int clamp_trigon(float x_min, float x_max, float y_min, float y_max,
		Trigon trigon, IVec2* new_vertices) {
	Vec2 a = trigon.a;
	Vec2 b = trigon.b;
	Vec2 c = trigon.c;

	Vec2 rect_start = {x_min, y_min};
	Vec2 rect_end = {x_max, y_max};

	int new_vertex_count = 0;

	// calculate ixn_points with screen borders, add to new_vertices
	Vec2 ixn_points[2];
	for (int i = 0; i < TRIGON_VERT_COUNT; i++) {
		int ixn_points_count =
				rect_line_intersect(rect_start, rect_end, trigon.v[i],
														trigon.v[(i + 1) % 3], ixn_points);
		for (int i = 0; i < ixn_points_count; i++) {
			new_vertices[new_vertex_count++] = get_IVec2(ixn_points[i]);
		}
	}

	// add vertices inside screen borders to new_vertices
	for (int i = 0; i < TRIGON_VERT_COUNT; i++) {
		if (rect_contains_point(rect_start, rect_end, trigon.v[i])) {
			new_vertices[new_vertex_count++] = get_IVec2(trigon.v[i]);
		}
	}

	// check if screen corner lies inside the triangle
	BaryCoords upper_left = get_bary_coords(a, b, c, (Vec2){x_min, y_min});
	if (upper_left.alpha > 0 && upper_left.beta > 0 && upper_left.gamma > 0) {
		new_vertices[new_vertex_count++] = (IVec2){x_min, y_min};
	}
	BaryCoords upper_right = get_bary_coords(a, b, c, (Vec2){x_max, y_min});
	if (upper_right.alpha > 0 && upper_right.beta > 0 && upper_right.gamma > 0) {
		new_vertices[new_vertex_count++] = (IVec2){x_max, y_min};
	}
	BaryCoords lower_left = get_bary_coords(a, b, c, (Vec2){x_min, y_max});
	if (lower_left.alpha > 0 && lower_left.beta > 0 && lower_left.gamma > 0) {
		new_vertices[new_vertex_count++] = (IVec2){x_min, y_max};
	}
	BaryCoords lower_right = get_bary_coords(a, b, c, (Vec2){x_max, y_max});
	if (lower_right.alpha > 0 && lower_right.beta > 0 && lower_right.gamma > 0) {
		new_vertices[new_vertex_count++] = (IVec2){x_max, y_max};
	}

	return new_vertex_count;
}

void draw_trigon(Renderer *r, Vec2 p0, Vec2 p1, Vec2 p2, uint32_t color) {
	Trigon trigon = new_Trigon(world_to_screen(&r->viewport, p0),
														 world_to_screen(&r->viewport, p1),
														 world_to_screen(&r->viewport, p2));

	// define screen borders
	float x_min = 200.0f;
	float x_max = (float)r->pixelbuffer.width - 200.0f;
	float y_min = 200.0f;
	float y_max = (float)r->pixelbuffer.height - 200.0f;

	// there can only ever be 7 points when cutting the trigon
	const int new_vertices_max = 9;
	IVec2 new_vertices[new_vertices_max];
	int new_vertex_count = clamp_trigon(x_min, x_max, y_min, y_max,
		trigon, new_vertices);

	// return early if no trigon can be formed
	if (new_vertex_count < 3) { return; }

	// --- debug ---
	// for (int i = 0; i < new_vertex_count; i++) {
	// 	draw_rect(r, add_Vec2(get_Vec2(new_vertices[i]), (Vec2){3, 3}),
	// 						get_Vec2(sub_IVec2(new_vertices[i], (IVec2){3, 3})), 0xFF00FF00);
	// }
	//
	// draw_rect(r, add_Vec2(p0, (Vec2){3, 3}),
	// 					sub_Vec2(p0, (Vec2){3, 3}), 0xFFFF0000);
	// draw_rect(r, add_Vec2(p1, (Vec2){3, 3}),
	// 					sub_Vec2(p1, (Vec2){3, 3}), 0xFFFF0000);
	// draw_rect(r, add_Vec2(p2, (Vec2){3, 3}),
	// 					sub_Vec2(p2, (Vec2){3, 3}), 0xFFFF0000);

	const int trigons_max = 5;
	ITrigon trigons[trigons_max];
	int trigon_count = 0;

	// ---- split quad into trigons ----
	if (new_vertex_count > 3) {
		Vec2 centeroid = {};
		for (int i = 0; i < new_vertex_count; i++) {
			centeroid.x += new_vertices[i].x;
			centeroid.y += new_vertices[i].y;
		}
		centeroid = mul_Vec2(centeroid, 1.0f/new_vertex_count);

		IndexAngle index_angles[new_vertex_count];
		for (int i = 0; i < new_vertex_count; i++) {
			Vec2 v = sub_Vec2(get_Vec2(new_vertices[i]), centeroid);
			index_angles[i] = (IndexAngle){i, atan2(v.y, v.x)};
		}

		qsort(index_angles, new_vertex_count, sizeof(IndexAngle), compare_angles);

		for (int i = 0; i < new_vertex_count - 2; i++) {
			trigons[trigon_count++] = new_ITrigon(new_vertices[index_angles[0].index],
				new_vertices[index_angles[i+1].index], new_vertices[index_angles[i+2].index]);
		}
	} else {
		trigons[trigon_count++] = new_ITrigon(new_vertices[0], new_vertices[1], new_vertices[2]);
	}

	// ---- draw all trigons ----
	for (int i = 0; i < trigon_count; i++) {
		ITrigon trigon = trigons[i];

		// early continue if trigon has no area
		if ((trigon.a.x == trigon.b.x && trigon.a.y == trigon.b.y) ||
				(trigon.a.x == trigon.c.x && trigon.a.y == trigon.c.y) ||
				(trigon.b.x == trigon.c.x && trigon.b.y == trigon.c.y)) {
			continue;
		}

		// if allready flat, draw right away
		if (trigon.a.x == trigon.b.x ||
				trigon.a.x == trigon.c.x ||
				trigon.b.x == trigon.c.x ||
				trigon.a.y == trigon.b.y ||
				trigon.a.y == trigon.c.y ||
				trigon.b.y == trigon.c.y) {
			draw_lerp_line_trigon(&r->pixelbuffer, trigon.a,
														trigon.b, trigon.c, color);
		} else {
		// sort for smallest y coordinate
			do {
				IVec2 temp = trigon.a;
				if (temp.y > trigon.b.y) {
					trigon.a = trigon.b;
					trigon.b = temp;
				}

				if (trigon.c.y < trigon.b.y) {
					temp = trigon.b;
					trigon.b = trigon.c;
					trigon.c = temp;
				}
			} while (trigon.a.y > trigon.b.y || trigon.b.y > trigon.c.y);

			// split horizontally, draw both splits
			float k = (float)(trigon.b.y - trigon.a.y) /
								(trigon.c.y - trigon.a.y);
			IVec2 p3 = {(int)(round(trigon.a.x + k * 
								(trigon.c.x - trigon.a.x))), trigon.b.y};
			draw_lerp_line_trigon(&r->pixelbuffer, trigon.a,
														trigon.b, p3, color);
			draw_lerp_line_trigon(&r->pixelbuffer, trigon.c,
														trigon.b, p3, color);
		}
	}
}

void draw_thick_line(Renderer *r, Vec2 start, Vec2 end, float wd,
                     uint32_t color) {
	Vec2 line = sub_Vec2(end, start);

	Vec2 a = get_ortho_Vec2(line);
	norm_Vec2(&a);

	Vec2 p0 = add_Vec2(start, mul_Vec2(a, (wd/2.0f)));
	Vec2 p1 = sub_Vec2(p0, mul_Vec2(a, wd));
	Vec2 p2 = add_Vec2(p1, line);
	Vec2 p3 = add_Vec2(p0, line);

	// --- debug ---
	// draw_rect(r, add_Vec2(p0, (Vec2){3, 3}),
	// 					sub_Vec2(p0, (Vec2){3, 3}), 0xFFFF0000);
	// draw_rect(r, add_Vec2(p1, (Vec2){3, 3}),
	// 					sub_Vec2(p1, (Vec2){3, 3}), 0xFFFF0000);
	// draw_rect(r, add_Vec2(p2, (Vec2){3, 3}),
	// 					sub_Vec2(p2, (Vec2){3, 3}), 0xFFFF0000);
	// draw_rect(r, add_Vec2(p3, (Vec2){3, 3}),
	// 					sub_Vec2(p3, (Vec2){3, 3}), 0xFFFF0000);

	draw_trigon(r, p0, p1,p3, color);
	draw_trigon(r, p1, p2,p3, color);
}
