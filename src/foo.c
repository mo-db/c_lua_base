#include "foo.h"
void foo(App* app) {
	int width = app->my_renderer->pixelbuffer.width;
	int height = app->my_renderer->pixelbuffer.height;
	for (int i = 0; i < 500; i++) {
		float rand1 = SDL_randf();
		float rand2 = SDL_randf();
		float rand3 = SDL_randf();
		draw_thick_line(app->my_renderer, (Vec2){300, 300},
				(Vec2){rand1*width, rand2*height}, 4*rand3, 0xFF0000FF);
	}
}
