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


void process_events(State* state) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
       state->context.quit = true;
      break;

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
