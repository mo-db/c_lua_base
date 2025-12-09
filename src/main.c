#include "app.h"
#include "graphics.h"
#include "render.h"
#include "foo.h"
#include "rpn.h"
#include "hum_ds.h"

#include "core.h"

#define W 940
#define H 520

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
	printf("builders_len: %d, generators_len: %d\n", 
			SSET_LEN(lmanager.builders), SSET_LEN(lmanager.generators));

	reconfigure_system(app.state.L, &lmanager);

	// co_init(&app);


//	--- foo setup ---
	// Trigon trigons[N_TRIGONS];
	// for (int i = 0; i < N_TRIGONS; i++) {
	// 	float ran = SDL_randf();
	//
	// 	trigons[i] = (Trigon){(Vec2){0, 0},
	// 		(Vec2){(double)app.width, (double)app.height * ran},
	// 		(Vec2){(double)app.width * ran, (double)app.height}};
	//
	// 	for (int j = 0; j < TRIGON_VERT_COUNT; j++) {
	// 		// trigons[i].v[j] = (Vec2){(-SDL_randf() + 0.5) * ran * i,
	// 		// 	(-SDL_randf() + 0.5) * ran * i};
	//
	// 	}
	// }
	uint64_t now = SDL_GetPerformanceCounter();
	uint64_t last = SDL_GetPerformanceCounter();
	uint64_t count = 0;
	double elapsed_time = 0;

	double accum = 0;



	while (!app.state.context.quit) {
		last = now;
		now = SDL_GetPerformanceCounter();
		count = SDL_GetPerformanceFrequency();
		elapsed_time = ((double)(now-last)/count) * 1000;

		process_events(&app);
		query_input(&app.state);

		// update_lua_State(&app.state);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderClear(app.renderer);
		if (update_viewport(&app.state, &app.my_renderer->viewport)) {
			renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);
		}
		// renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);


		// reconfigure system

		accum += elapsed_time;
		if (accum > 500.0) {
			printf("---------------------HI\n");
			accum = 0;
			renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);
			lua_reload_file(app.state.L, "scripts/gramma_def.lua");
			reconfigure_system(app.state.L, &lmanager);
		}

		if (became_true(app.state.input.shift)) {
			renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);
			lua_reload_file(app.state.L, "scripts/gramma_def.lua");
			reconfigure_system(app.state.L, &lmanager);
		}

		if (became_true(app.state.input.ctrl)) {
			renderer_clear(&app.my_renderer->pixelbuffer, 0xFF000000);
			lua_reload_file(app.state.L, "scripts/gramma_def.lua");
			reconfigure_system(app.state.L, &lmanager);
			SSet_at(&lmanager.generators, 0)->iterations++;
		}

		

		bool out_of_time = update_lsystem(app.my_renderer, &lmanager, elapsed_time, now);

		// foo(&app, trigons);
		// double elapsed =
		// 	((double)(SDL_GetPerformanceCounter() - now) / SDL_GetPerformanceFrequency()) * 1000;
		//  printf("foo-time: %f\n", elapsed);



		// co_update(&app, elapsed_time);


		// printf("elapsed: %f\n", ((double)(now-last)/count) * 1000);


		// bar(&app);

		// printf("input: %s\n", app.state.text);

		// --- foo ---
		// uint64_t start = SDL_GetPerformanceCounter();
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

