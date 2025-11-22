#include "core.h"
#include "app.h"
#include "graphics.h"
#include "render.h"
#include "foo.h"

#define W 1920
#define H 1080

int main() {
	App app = {};
	app_init(&app, W, H);
	while (!app.state.context.quit) {
		process_events(&app.state);
		query_input(&app.state);
		update_viewport(&app.state, &app.my_renderer->viewport);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderClear(app.renderer);
		renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);

		uint64_t start = SDL_GetPerformanceCounter();
		foo(&app);
		uint64_t end = SDL_GetPerformanceCounter();
		uint64_t count = SDL_GetPerformanceFrequency();
		printf("elapsed: %f\n", ((double)(end-start)/count) * 1000);

		SDL_UpdateTexture(app.window_texture, nullptr, app.my_renderer->pixelbuffer.pixels,
											app.width * 4);
		SDL_RenderTexture(app.renderer, app.window_texture, NULL, NULL);
    SDL_RenderPresent(app.renderer);
	}
	return 0;
}
