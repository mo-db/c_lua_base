#include "app.h"
#include "graphics.h"
#include "render.h"
#include "foo.h"
#include "rpn.h"
#include "hum_ds.h"

#include "core.h"

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

	LManager lmanager = LManager_new();

	// reload lua lsystem config
	lua_reload_file(app.state.L, "scripts/gramma_def.lua");
	lua_register_function(app.state.L, ladd_generator, "_add_generator");
	lua_register_function(app.state.L, lremove_generator, "_remove_generator");
	lua_register_function(app.state.L, ladd_builder, "_add_builder");
	lua_register_function(app.state.L, lremove_builder, "_remove_builder");

	// calls the lua function
	LManager_init_from_config(app.state.L, &lmanager);

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


		TimeState gen_state = update_gen();

		bool out_of_time = false;
		switch (gen_state) {
			case IDLE:
				if (!reset_generator) {
					break;
				}
				do_reset_generator(); // reset_generator == false
				gen_state = WORKING;
			case WORKING:
				if (reset_generator) {
					do_reset_generator();
				}
				if (!expand()) {
					out_of_time = true;
					break;
				}
				// mark all registered interpreters for reset
				for (size_t i = 0; i < SSET_LEN(inters); i++) {
					Interpreter *inter = SSet_at(&inters, i);
					if (inter->bind_to_generator) {
						inter->view = gen.expanded_string;
						inter->reset_needed = true;
					}
				}
				gen_state = IDLE;
				break;
			default: EXIT();
		}

		typedef enum {
			INTER_IDLE,
			BUILDING,
		} InterState;

		TimeState inter_state = IDLE;

		for (size_t i = 0; i < SSET_LEN(inters); i++) {
			if (!out_of_time) {
				switch(inter_state) {
					case IDLE:
						if (!inter_reset_needed) {
							break;
						}
						do_reset_inter(); // inter_reset_needed = false
						inter_state = WORKING;	
					case WORKING:
						if (inter_reset_needed) {
							do_reset_inter();
						}
						if (!build()) {
							out_of_time = true;
							break;
						}
						redraw_all = true;
						inter_state = IDLE;
						break;
					default: EXIT();
				}
			}

			// draw constructs into buffer
			if (!out_of_time) {
				// need to redraw_all objects if i zoom or pan or change pos
				if (redraw_all) {
				}
				if (!draw_inter()) {
					out_of_time = true;
				} else {
					draw_completed_count = (draw_completed_count + 1) % SSET_LEN(inters);
				}

				switch (drawer_state) {
					case IDLE: 
						if (!redraw_all) {
							break;
						}
						do_reset_drawer(); // = false
						drawer_state = WORKING;	
					case WORKING:
						if (redraw_all) {
							do_reset_drawer();
						}
						if (!draw_construct()) {
							out_of_time = true;
							break;
						}
						drawer_state = IDLE;
						break;
					default: EXIT();
				}
			}
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

