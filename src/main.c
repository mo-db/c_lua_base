#include "core.h"
#include "app.h"
#include "graphics.h"
#include "render.h"

#define W 640
#define H 480

int main() {
	App app = {};
	app_init(&app, W, H);
	while (!app.state.context.quit) {
		process_events(&app.state);
		query_input(&app.state);

		int mouse_x = app.state.input.mouse.x;
		int mouse_y = app.state.input.mouse.y;

		// update_viewport(app.state, *app.video.viewport);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderClear(app.renderer);
		renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);

		Vec2 a = (Vec2){50, 300};
		Vec2 b = (Vec2){700, 50};
		Vec2 c = (Vec2){200, 500};

		// get_bary_coords(a, b, c, app.state.input.mouse);
		// draw_lerp_line(app.my_renderer, a, b, 0xFF0000FF);
		// draw_lerp_line(app.my_renderer, a, c, 0xFF0000FF);
		// draw_lerp_line(app.my_renderer, b, c, 0xFF0000FF);

		// draw_rect(app.my_renderer, (Vec2){100, 100}, app.state.input.mouse, 0xFF0000FF);
		// draw_thick_line(app.my_renderer, (Vec2){100, 100}, app.state.input.mouse, 10.0, 0xFF0000FF);

		draw_trigon(app.my_renderer, a, b, app.state.input.mouse, 0xFF0000FF);
		SDL_UpdateTexture(app.window_texture, nullptr,
											app.my_renderer->pixelbuffer.pixels, app.width * 4);
		SDL_RenderTexture(app.renderer, app.window_texture, NULL, NULL);

		printf("ctrl down! %d\n", get_state(app.state.input.ctrl));
		printf("mouse: %d, %d\n", mouse_x, mouse_y);
		if (get_state(app.state.input.ctrl)) {
		}
    SDL_RenderPresent(app.renderer);
	}
	return 0;
}
