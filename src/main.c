#include "core.h"
#include "app.h"
#include "graphics.h"
#include "render.h"
#include "foo.h"
#include "rpn.h"

#define W 1080
#define H 720

// bei context-sensitive languages gibt es kein verfahren
// welches entscheiden kann ob ein wort in der sprache ist
// -> bei context-frei gibt es den CYK algorithmus
// -> deswegen pumpinglemma 2 zur unterscheidung

int main() {
	App app = {};
	app_init(&app, W, H);

	app.state.L = reload_lua();

	Generator gen = new_generator();

	InterpreterState istate = {};
	istate.pos = (Vec2){(float)app.width/2, (float)app.height/2};
	istate.angle = PI / 2;
	istate.width = 5.0;
	istate.queue_head = 0;
	Interpreter inter = new_interpreter(gen.expanded_string, istate);

	configure_generator(&app, &gen, &inter);

	// co_init(&app);


	// --- foo setup ---
	// Trigon trigons[N_TRIGONS];
	// for (int i = 0; i < N_TRIGONS; i++) {
	// 	float ran = SDL_randf() * 500;
	// 	for (int j = 0; j < TRIGON_VERT_COUNT; j++) {
	// 		trigons[i].v[j] = (Vec2){(-SDL_randf() + 0.5) * ran * i,
	// 			(-SDL_randf() + 0.5) * ran * i};
	// 	}
	// }
	
	uint64_t now = SDL_GetPerformanceCounter();
	uint64_t last = SDL_GetPerformanceCounter();
	uint64_t count = 0;
	double elapsed_time = 0;



	while (!app.state.context.quit) {
		last = now;
		now = SDL_GetPerformanceCounter();
		count = SDL_GetPerformanceFrequency();
		elapsed_time = ((double)(now-last)/count) * 1000;

		process_events(&app);
		query_input(&app.state);
		update_viewport(&app.state, &app.my_renderer->viewport);
		// update_lua_State(&app.state);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderClear(app.renderer);
		renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);

		if (became_true(app.state.input.shift)) {
			reload_generator_config(app.state.L);
			configure_generator(&app, &gen, &inter);
			gen.reset_needed = true;
		}

		if (became_true(app.state.input.ctrl)) {
			gen.iterations++;
			gen.done_generating = false;
		}

		inter.reset_needed = update_generator(&gen);

		inter.view = gen.expanded_string;
		inter.redraw_needed = update_inter(&inter);
		if (inter.done_building) {
			gen_draw_timed(&inter, &app);
		}


		// co_update(&app, elapsed_time);



		// printf("elapsed: %f\n", ((double)(now-last)/count) * 1000);




		// bar(&app);

		// printf("input: %s\n", app.state.text);

		// --- foo ---
		// uint64_t start = SDL_GetPerformanceCounter();
		// foo(&app, trigons);
		// uint64_t end = SDL_GetPerformanceCounter();
		// uint64_t count = SDL_GetPerformanceFrequency();
		// printf("elapsed: %f\n", ((double)(end-start)/count) * 1000);

		// --- *** SDL flip screen texture *** ---
		SDL_UpdateTexture(app.window_texture, NULL, app.my_renderer->pixelbuffer.pixels,
											app.width * 4);
		SDL_RenderTexture(app.renderer, app.window_texture, NULL, NULL);
    SDL_RenderPresent(app.renderer);
	}
	return 0;
}

