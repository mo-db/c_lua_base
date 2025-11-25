#include "core.h"
#include "app.h"
#include "graphics.h"
#include "render.h"
#include "foo.h"
#include "regex.h"

#define W 640
#define H 480

int main() {
	App app = {};
	app_init(&app, W, H);

	app.state.L = core_lua_dofile("scripts/gramma_def.lua");

	char* pattern = "a*b";
	char* subject = "ccccaxxbccccabcaxbcccc";

	regex_match(pattern, subject);
	return 0;


	// --- foo setup ---
	Trigon trigons[N_TRIGONS];
	for (int i = 0; i < N_TRIGONS; i++) {
		float ran = SDL_randf() * 500;
		for (int j = 0; j < TRIGON_VERT_COUNT; j++) {
			trigons[i].v[j] = (Vec2){(-SDL_randf() + 0.5) * ran * i,
				(-SDL_randf() + 0.5) * ran * i};
		}
	}
	
	while (!app.state.context.quit) {
		process_events(&app);
		query_input(&app.state);
		update_viewport(&app.state, &app.my_renderer->viewport);
		// update_lua_State(&app.state);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderClear(app.renderer);
		renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);

		bar(&app);

		// printf("input: %s\n", app.state.text);

		// --- foo ---
		// uint64_t start = SDL_GetPerformanceCounter();
		// foo(&app, trigons);
		// uint64_t end = SDL_GetPerformanceCounter();
		// uint64_t count = SDL_GetPerformanceFrequency();
		// printf("elapsed: %f\n", ((double)(end-start)/count) * 1000);

		SDL_UpdateTexture(app.window_texture, NULL, app.my_renderer->pixelbuffer.pixels,
											app.width * 4);
		SDL_RenderTexture(app.renderer, app.window_texture, NULL, NULL);
    SDL_RenderPresent(app.renderer);
	}
	return 0;
}
