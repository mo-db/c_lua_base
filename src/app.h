// app.hpp
#pragma once

#include "core.h"
#include "graphics.h"
#include "render.h"

// a generic state, state(bool state) eneds to be called every frame
class GenericState {
  bool current_state{false};
  bool previous_state{false};

public:
  bool state() { 
		return current_state; 
	}
  void state(bool state) { 
		previous_state = current_state;
		current_state = state; 
	}
  bool became_true() {
    return (previous_state == false && current_state == true) ? true : false;
  }
  bool became_false() {
    return (previous_state == true && current_state == false) ? true : false;
  }
};


// every frame query_input() needs to call down() for every input_state
class InputState {
  bool current_state{false};
  bool prev_state{false};

public:
  bool down() { 
		return current_state; 
	}
  void down(bool state) { 
		prev_state = current_state;
		current_state = state; 
	}
  bool just_pressed() {
    return (prev_state == false && current_state == true) ? true : false;
  }
  bool just_released() {
    return (prev_state == true && current_state == false) ? true : false;
  }
};

struct State {
	lua_State *L;

	struct Context {
		bool quit{false};
	} context;

	struct Video {
		GenericState panning{};
		GenericState scaling{};
	} video;

	struct Input {
		InputState mouse_left{};
		InputState mouse_mid{};
		InputState mouse_right{};
		InputState shift{};
		InputState ctrl{};
		InputState left{};
		InputState right{};
		InputState up{};
		InputState down{};
		Vec2 mouse{};
	} input;
};

// big structure that has access to everything
struct App {
  SDL_Window* window{};
	SDL_Renderer* renderer{};
	SDL_Texture* window_texture{};

	int pixel_density{};
	int width{};
	int height{};

	struct Video {
		std::unique_ptr<render::PixelBuffer> pixel_buffer{};
		std::unique_ptr<render::Viewport> viewport{};
	} video;


	State state;
};

// run once at startup after creating the app object
bool app_init(App& app, int width, int height);

// run every frame
void process_events(State& state);

// run every frame
void query_input(State& state);

// update the render viewport using input
void update_viewport(State& state, render::Viewport& viewport);
