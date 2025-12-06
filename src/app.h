// app.h
#pragma once

#include "core.h"
#include "graphics.h"
#include "render.h"

#include "coroutines.h"

// a generic state, state(bool state) eneds to be called every frame
typedef struct GenericState {
  bool current_state;
  bool prev_state;
} GenericState;

bool get_state(const GenericState is);
void set_state(GenericState* is, const bool state);
bool became_true(const GenericState is);
bool became_false(const GenericState is);


typedef struct Context {
	bool quit;
} Context;

typedef struct Video {
	GenericState panning;
	GenericState scaling;
} Video;

typedef struct Input {
	GenericState mouse_left;
	GenericState mouse_mid;
	GenericState mouse_right;
	GenericState shift;
	GenericState ctrl;
	GenericState left;
	GenericState right;
	GenericState up;
	GenericState down;
	GenericState w;
	GenericState s;
	GenericState a;
	GenericState d;

	Vec2 mouse;
} Input;

typedef struct State {
	Context context;
	Video video;
	Input input;
	lua_State *L;
	CoState co;

	char text[1024];
	bool text_input_complete;


} State;

// big structure that has access to everything
typedef struct App {
  SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* window_texture;
	Renderer* my_renderer;


	int pixel_density;
	int width;
	int height;

	State state;
} App;

// run once at startup after creating the app object
bool app_init(App* app, int width, int height);

// run every frame
void process_events(App* app);

// run every frame
void query_input(State* state);

// update the render viewport using input
void update_viewport(State* state, Viewport* viewport);

// lua
// void update_lua_State(State* state);
lua_State* reload_lua();
void reload_generator_config(lua_State* L);
