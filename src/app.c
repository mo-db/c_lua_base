#include "app.h"


bool get_state(const GenericState is) { 
	return is.current_state; 
}
void set_state(GenericState* is, const bool state) { 
	is->prev_state = is->current_state;
	is->current_state = state; 
}
bool became_true(const GenericState is) {
	return (is.prev_state == false && is.current_state == true) ? true : false;
}
bool became_false(const GenericState is) {
	return (is.prev_state == true && is.current_state == false) ? true : false;
}


bool app_init(App* app, int width, int height) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return false;
  }
  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_HIGH_PIXEL_DENSITY |
                                 SDL_WINDOW_MOUSE_CAPTURE;

  app->window = SDL_CreateWindow("main", width, height, window_flags);
  if (!app->window) {
    return false;
  }
  app->renderer = SDL_CreateRenderer(app->window, NULL);
  if (!app->renderer) {
    return false;
  }
  SDL_SetRenderVSync(app->renderer, 1);
  SDL_SetWindowPosition(app->window, SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);

	SDL_GetWindowSizeInPixels(app->window, &app->width, &app->height);
  // app->width = width;
  // app->height = height;
	app->pixel_density = 1;

  app->window_texture =
      SDL_CreateTexture(app->renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_TARGET, app->width, app->height);

	// test

	app->my_renderer = new_renderer(app->width, app->height);
	renderer_clear(&app->my_renderer->pixelbuffer, 0xFF000000);

	// --- lua ---
	app->state.L = luaL_newstate();
	luaL_openlibs(app->state.L);

  return true;
}


void process_events(App* app) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_EVENT_QUIT:
       app->state.context.quit = true;
      break;
		case SDL_EVENT_TEXT_INPUT:
      	SDL_strlcat(app->state.text, e.text.text, sizeof(app->state.text));
				break;
		case SDL_EVENT_KEY_DOWN:
				if (e.key.key == SDLK_ESCAPE || e.key.key == SDLK_RETURN) {
						SDL_StopTextInput(app->window);
						app->state.text_input_complete = true;
				} else if (e.key.key == SDLK_I) {
						SDL_StartTextInput(app->window);
						app->state.text_input_complete = false;
				}
		default: break;
		}
	}
}


void query_input(State* state) {
	SDL_MouseButtonFlags mouse_flags =
		SDL_GetMouseState(&state->input.mouse.x, &state->input.mouse.y);
	state->input.mouse.x *= 2;
	state->input.mouse.y *= 2;

	set_state(&state->input.mouse_left, mouse_flags & SDL_BUTTON_LMASK);
	set_state(&state->input.mouse_mid, mouse_flags & SDL_BUTTON_MMASK);
	set_state(&state->input.mouse_right, mouse_flags & SDL_BUTTON_RMASK);

	const bool* key_states = SDL_GetKeyboardState(NULL);
	set_state(&state->input.shift, key_states[SDL_SCANCODE_LSHIFT]);
	set_state(&state->input.ctrl, key_states[SDL_SCANCODE_LCTRL]);
	set_state(&state->input.left, key_states[SDL_SCANCODE_LEFT]);
	set_state(&state->input.right, key_states[SDL_SCANCODE_RIGHT]);
	set_state(&state->input.up, key_states[SDL_SCANCODE_UP]);
	set_state(&state->input.down, key_states[SDL_SCANCODE_DOWN]);

	set_state(&state->input.w, key_states[SDL_SCANCODE_W]);
	set_state(&state->input.s, key_states[SDL_SCANCODE_S]);
	set_state(&state->input.a, key_states[SDL_SCANCODE_A]);
	set_state(&state->input.d, key_states[SDL_SCANCODE_D]);
}


void update_viewport(State* state, Viewport* viewport) {
	if (get_state(state->input.left)) {
		viewport->xy_offset.x += (4 / viewport->scale);
	}
	else if (get_state(state->input.right)) {
		viewport->xy_offset.x -= (4 / viewport->scale);
	}
	else if (became_true(state->input.up)) {
		viewport->scale *= 0.75;
	}
	else if (became_true(state->input.down)) {
		viewport->scale *= 1.25;
	}
}

// void update_lua_State(State* state) {
// 	if (became_true(state->input.shift)) {
// 		state->L = core_lua_dofile("scripts/test.lua");
// 	}
// }


lua_State* reload_lua() {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	char* scripts[] = { "scripts/gramma_def.lua", "scripts/coroutines.lua" };
	size_t n_files = sizeof(scripts) / sizeof(char*);

	lua_pushcfunction(L, wrap_lua_create_level);
	lua_setglobal(L, "wrap_lua_create_level");

	lua_pushcfunction(L, lua_create_dynamic_object);
	lua_setglobal(L, "lua_create_dynamic_object");

	lua_pushcfunction(L, lua_assign_player_control);
	lua_setglobal(L, "lua_assign_player_control");


	lua_pushcfunction(L, lua_move_object);
	lua_setglobal(L, "lua_move_object");


	for (size_t i = 0; i < n_files; i++) {
		printf("script: %s\n", scripts[i]);
		if (lua_check(L, luaL_dofile(L, scripts[i]))){
			printf("FUU\n");
		} else {
			printf("BUU\n");
			EXIT();
		}
	}
	return L;
}


