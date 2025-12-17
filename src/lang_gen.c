#include "lang_gen.h"

/* --- LManager --- */
LManager *LManager_new() {
	LManager *manager = malloc(sizeof(LManager));
	manager->generators = SPSet_new();
	manager->builders = SPSet_new();
	return manager;
}
void LManager_free(LManager* manager) {
	if (!manager) { EXIT(); }
	for (int i = 0; i < DS_LEN(manager->generators); i++) {
		generator_free(SPSet_at(manager->generators, i));
	}
	for (int i = 0; i < DS_LEN(manager->builders); i++) {
		builder_free(SPSet_at(manager->builders, i));
	}
	SPSet_free(&manager->generators);
	SPSet_free(&manager->builders);
	free(manager);
}

/* --- LManager config from lua --- */
void LManager_init_from_config(lua_State* L, LManager* manager) {
	lua_getglobal(L, "lmanager_init");
	if (!lua_isfunction(L, -1)) {
		EXIT();
	}
	lua_pushlightuserdata(L, manager);
	if (!lua_check(L, lua_pcall(L, 1, 1, 0))) {
		EXIT();
	}
}

void configure_defaults(lua_State *L, Generator *generator) {
	double value = 0;
	if (lua_table_get_number(L, "move", &value)) { 
		generator->move_default = value;
	}
	if (lua_table_get_number(L, "rotate", &value)) {
		generator->rotate_default = value;
	}
}

static void configure_globals(lua_State *L, Generator *generator) {
  double value = 0;
  if (lua_table_get_number(L, "h", &value)) {
		SSet_emplace_id(generator->vars, 'h', value);
	}
  if (lua_table_get_number(L, "i", &value)) {
		SSet_emplace_id(generator->vars, 'i', value);
	}
  if (lua_table_get_number(L, "j", &value)) {
		SSet_emplace_id(generator->vars, 'j', value);
	}
  if (lua_table_get_number(L, "k", &value)) {
		SSet_emplace_id(generator->vars, 'k', value);
	}
}

static void configure_productions(lua_State *L, Generator *generator) {
	// free productions and clear the SPSet
	for (int i = 0; i < DS_LEN(generator->productions); i++) {
		production_free(SPSet_at(generator->productions, i));
	}
	SPSet_clear(generator->productions);
	// readd productions
	Str *production_str = Str_new();
	for (int i = 1; lua_table_string_at(L, i, production_str); i++) {
		Production *prod = parse_production_str(production_str);
		// format_production(generator, prod);
		if (SPSet_push_back(generator->productions, prod) == UINT32_MAX) { EXIT(); }
		Str_clear(production_str);
		// printf("--- PROD DEBUG PRINT ---\n");
		// Str_print(prod->symbol);
		// Str_print(prod->condition);
		// Str_print(prod->context);
		// Str_print(prod->replacement);
		// printf("--- DEBUG PRINT END---\n");
	}
	Str_free(&production_str);
}

typedef enum {
	DEFAULT,
	SPECIFIC
} ConfigType;

// bool lua_table_empty(lua_State *L) {
// 		lua_pushnumber(L, 1);
// 		lua_gettable(L, -2);
// 		if (lua_isnil(L, -1)) {	
// 			lua_pop(L, 1);
// 			return true;
// 		}
// 		lua_pop(L, 1);
// 		return false;
// }

void configure_generator(lua_State *L, Generator *generator) {
	// get table-field defaults
	lua_pushstring(L, "defaults");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		if (!lua_table_empty(L, -1)) {
			configure_defaults(L, generator);
		}
	}
	lua_pop(L, 1);

	// get table-field globals
	lua_pushstring(L, "globals");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) { 
		if (!lua_table_empty(L, -1)) {
			configure_globals(L, generator);
		}
	}
	lua_pop(L, 1);

	// get table-field productions
	lua_pushstring(L, "productions");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) {
		if (!lua_table_empty(L, -1)) {
			configure_productions(L, generator);
		}
	}

	lua_pop(L, 1);
}

void configure_builder(lua_State *L, Builder *builder) {
	double value = 0;
	// get table-field pos
	lua_pushstring(L, "pos");
	lua_gettable(L, -2);
	if (lua_istable(L, -1)) { 
		if (lua_table_number_at(L, 1, &value)) { 
			builder->start_state.pos.x = value;
		}
		if (lua_table_number_at(L, 2, &value)) {
			builder->start_state.pos.y = value;
		}
	}
	lua_pop(L, 1);

	// get other fields
	if (lua_table_get_number(L, "angle", &value)) { 
		builder->start_state.angle = value;
	}
	if (lua_table_get_number(L, "segment_node_count", &value)) {
		if (value < 1 || value > 3) {
			builder->segment_node_count = 1;
		} else {
			builder->segment_node_count = value;
		}
	}
	if (lua_table_get_number(L, "generator_id", &value)) {
		builder->generator_id = value;
	}
}

void reconfigure_system(lua_State *L, LManager *manager) {
	lua_getglobal(L, "generator_configs");
	if (!lua_istable(L, -1)) { EXIT(); }
	for (size_t i = 0; i < DS_LEN(manager->generators); i++) {
		Generator *generator = SPSet_at(manager->generators, i);
		uint32_t generator_id = SPSet_id_at(manager->generators, i);
		generator->reset_needed = true;

		// configure with default config
		lua_pushstring(L, "default");
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) { EXIT(); }
		configure_generator(L, generator);
		lua_pop(L, 1);
		// overwrite with specific config if present
		lua_pushnumber(L, generator_id);
		lua_gettable(L, -2);
		if (lua_istable(L, -1)) {
			configure_generator(L, generator);
		}
		lua_pop(L, 1);
	}

	// format the prductions to include the defaults
	for (size_t i = 0; i < DS_LEN(manager->generators); i++) {
		Generator *generator = SPSet_at(manager->generators, i);
		for (size_t i = 0; i < DS_LEN(generator->productions); i++) {
			format_production(generator, SPSet_at(generator->productions, i));
		}
	}

	lua_getglobal(L, "builder_configs");
	if (!lua_istable(L, -1)) { EXIT(); }
	for (size_t i = 0; i < DS_LEN(manager->builders); i++) {
		Builder *builder = SPSet_at(manager->builders, i);
		uint32_t builder_id = SPSet_id_at(manager->builders, i);
		builder->reset_needed = true;

		// configure with default config
		lua_pushstring(L, "default");
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) { EXIT(); }
		configure_builder(L, builder);
		lua_pop(L, 1);
		// overwrite with specific config if present
		lua_pushnumber(L, builder_id);
		lua_gettable(L, -2);
		if (lua_istable(L, -1)) {
			configure_builder(L, builder);
		}
		lua_pop(L, 1);
	}
}

/* --- general lsystem stuff --- */
void redraw_all(LManager *manager) {
	for (size_t i = 0; i < DS_LEN(manager->builders); i++) {
		Builder *builder = SPSet_at(manager->builders, i);
		builder->redraw_needed = true;
		builder->current_construct_index = 0;
	}
}

SymbolCategory get_symbol_category(char ch) {
	if (isalpha(ch)) {
		return Move;
	}
	if (ch == '-' || ch == '+') {
		return Rotate;
	}
	if (ch == '^' || ch == '&') {
		return Width;
	}
	if (ch == '$' || ch == '%') {
		return Color;
	}
	if (ch == '[' || ch == ']') {
		return Stack;
	}
	EXIT();
}

double get_default(Generator* gen, SymbolCategory cat) {
	double value = {};
	switch (cat) {
		case Move: value = gen->move_default; break;
		case Rotate: value = gen->rotate_default; break;
		default: break;
	}
	return value;
}

/* --- lua-registered functions --- */
int ladd_generator(lua_State* L) {
	LManager* manager = lua_touserdata(L, 1);
	if (!manager) { EXIT(); }
	uint32_t id = add_generator(manager);
	if (id == UINT32_MAX) { EXIT(); }
	lua_pushnumber(L, id);
	return 1;
}
int lremove_generator(lua_State* L) {
	LManager* manager = lua_touserdata(L, 1);
	if (!manager) { EXIT(); }
	uint32_t id = luaL_checknumber(L, 2);	
	if (!remove_generator(manager, id)) {
		lua_pushnumber(L, 0);
	}
	lua_pushnumber(L, 1);
	return 1;
}
int ladd_builder(lua_State* L) {
	LManager* manager = lua_touserdata(L, 1);
	if (!manager) { EXIT(); }
	uint32_t id = add_builder(manager);
	if (id == UINT32_MAX) { EXIT(); }
	lua_pushnumber(L, id);
	return 1;
}
int lremove_builder(lua_State* L) {
	LManager* manager = lua_touserdata(L, 1);
	if (!manager) { EXIT(); }
	uint32_t id = luaL_checknumber(L, 2);	
	if (!remove_builder(manager, id)) {
		lua_pushnumber(L, 0);
	}
	lua_pushnumber(L, 1);
	return 1;
}

int lget_builder_pos(lua_State* L);
int lget_builder_angle(lua_State* L);
int lget_builder_nodes(lua_State* L);

/* --- lstring generation --- */
Generator *generator_new() {
	Generator *gen = calloc(1, sizeof(Generator));
	gen->vars = SSet_new();
	gen->productions = SPSet_new();
	gen->replacement_buffer = Str_new();
	gen->str0 = Str_new();
	gen->str1 = Str_new();
	gen->expanded_string = gen->str0;
	gen->iterations = 1;
	return gen;
}

void generator_free(Generator *gen) {
	if (!gen) { EXIT(); }
	SSet_free(&gen->vars);
	for (int i = 0; i < DS_LEN(gen->productions); i++) {
		production_free(SPSet_at(gen->productions, i));
	}
	SSet_free(&gen->productions);
	Str_free(&gen->replacement_buffer);
	Str_free(&gen->str0);
	Str_free(&gen->str1);
	free(gen);
}

void reset_generator(Generator* gen) {
		gen->current_iteration = 0;
		gen->current_index = 0;
		Str_clear(gen->replacement_buffer);
		Str_clear(gen->str0);
		Str_clear(gen->str1);
		gen->expanded_string = gen->str0;
		gen->reset_needed = false;
}

uint32_t add_generator(LManager* manager) {
	Generator *gen = generator_new();
	uint32_t id = SPSet_push_back(manager->generators, gen);
	return id;
}

bool remove_generator(LManager* manager, uint32_t id) {
	Generator *gen = SPSet_get(manager->generators, id);
	generator_free(gen);
	return SPSet_remove(manager->generators, id);
}

void clear_generators(LManager* manager) {
	SPSet_clear(manager->generators);
}

Production *production_new() {
	Production *production = malloc(sizeof(Production));
	production->symbol = Str_new();
	production->condition = Str_new();
	production->context = Str_new();
	production->replacement = Str_new();
	return production;
}

void production_free(Production *production) {
	if (!production) { EXIT(); }
	Str_free(&production->symbol);
	Str_free(&production->condition);
	Str_free(&production->context);
	Str_free(&production->replacement);
	free(production);
}

bool arg_block_empty(StrView arg_block) {
	if (*arg_block.data != '{') { EXIT(); }

	while (*(++arg_block.data) != '}')
		if (*arg_block.data != ' ') {
			return false;
		}
	return true;
}

// find delim in LS
bool find_delim(StrView str, char delim, uint32_t* pos) {
	for (uint32_t i = 0; i < str.len; i++) {
		if (str.data[i] == delim) {
			*pos = i;
			return true;
		}
	}
	*pos = str.len;
	return false;
}

void format_production(Generator *generator, Production *production) {
	Str *replacement = production->replacement;
	Str *tmp = Str_new();
	for (int i = 0; i < replacement->len; i++) {
		Str_putc(tmp, replacement->data[i]);
	}

	StrView production_view = Str_get_view(tmp);
	char *view_end = production_view.data + production_view.len;

	Str_clear(replacement);

	// fill in defaults
	while (production_view.data < view_end) {
		char symbol = *production_view.data;

		if (symbol == ' ') {
			if (!StrView_offset(&production_view, 1)) { EXIT(); }
			continue;
		}

		if (symbol == '[' || symbol == ']') {
			if (!Str_putc(replacement, symbol)) { EXIT(); }
			if (!StrView_offset(&production_view, 1)) { EXIT(); }
			continue;
		}

		if (production_view.data + 1 < view_end) {
			SymbolCategory symbol_category = get_symbol_category(symbol);
			if (!Str_putc(replacement, symbol)) { EXIT(); }
			if (!StrView_offset(&production_view, 1)) { EXIT(); }

			if (*production_view.data != '{') {
				double default_value = get_default(generator, symbol_category);
				// this sucks
				char new_arg_block[64];
				new_arg_block[63] = '\0';
				snprintf(new_arg_block, 64, "{%f}", default_value);
				if (Str_put_view(replacement, Str_get_view_cstr(new_arg_block)) == 0) { EXIT(); }
			} else {
				StrView block = {};
				if (!get_block(production_view, '{', &block)) { EXIT(); };
				if (arg_block_empty(block)) {
					double default_value = get_default(generator, symbol_category);
					// sucks also
					char new_arg_block[64];
					new_arg_block[63] = '\0';
					snprintf(new_arg_block, 64, "{%f}", default_value);
					if (Str_put_view(replacement, Str_get_view_cstr(new_arg_block)) == 0) { EXIT(); }
				} else {
					if (Str_put_view(replacement, block) == 0) { EXIT(); }
				}
				if (!StrView_offset(&production_view, block.len)) { EXIT(); }
			}
		}
	}

	Str_free(&tmp);
}

// S{} : condition : context ! replacement
Production *parse_production_str(Str* production_str) {
	Production *production = production_new();

	// replacement
	uint32_t seperator_position = 0;
	StrView production_view = Str_get_view(production_str);
	if (!find_delim(production_view, '!', &seperator_position)) { EXIT(); }
	if (!StrView_offset(&production_view, seperator_position + 1)) { EXIT(); }
	Str_put_view(production->replacement, production_view);

	// symbol
	production_view = Str_get_view(production_str);
	production_view.len = seperator_position;
	if (!find_delim(production_view, ':', &seperator_position)) { 
		Str_put_view(production->symbol, production_view);
		return production;
	}
	Str_put_view(production->symbol,
			(StrView){production_view.data, seperator_position});

	// condition
	if (!StrView_offset(&production_view, seperator_position + 1)) { EXIT(); }
	if (!find_delim(production_view, ':', &seperator_position)) { 
		Str_put_view(production->condition, production_view);
		return production;
	}
	Str_put_view(production->condition,
			(StrView){production_view.data, seperator_position});
	// context
	if (!StrView_offset(&production_view, seperator_position + 1)) { EXIT(); }
		Str_put_view(production->context, production_view);

	return production;
}

// evaluate production into replacement cache
// replacement has been formated to:
// [S{x,y,z} or not ? do i need defaults, i think i do, scaling vals
// first try without args
bool eval_and_append_replacement(Generator* gen, Str *dest, Str *replacement) {
	if (!gen || !dest || !replacement) { EXIT(); }
	// --- fill replacement buffer evaluating the replacement ---
	// first reset it
	gen->replacement_buffer->len = 0;
	if (!Str_put_view(gen->replacement_buffer, Str_get_view(replacement))) {
		EXIT();
	}

	// complex later
	// int new_prod_index = 0;	
	// int prod_index = 0;	
	// bool result = true;
	// while (prod_index < replacement.len) {
		//result = get_arg_block();	

		// parse into strings
		//result = parse_arg_block();

		// arraylist of n doubles 
		//result = eval_args();

		// combine into evaluated arg block string
		//snprintf()
		// copy onto prod_cache
	// }
	
	// return false if replacement couldnt do
	
	// --- copy the evaluated replacement from the buffer
	if (!Str_put_view(dest, Str_get_view(gen->replacement_buffer))) { EXIT(); }
	return true;
}

// TODO: this needs change, maybe_fill_replacement_string
Str *maybe_replace(Generator* gen, char symbol, StrView block) {
	if (block.len <= 2) {
		// no args
	}
	for (uint32_t i = 0; i < DS_LEN(gen->productions); i++) {
		Production* prod = SPSet_at(gen->productions, i);
		if (*prod->symbol->data != symbol) { continue; }

		// check condition

		// check context

		return prod->replacement;
	}
	return NULL;
}

// get pobuilder to start and end of {} block
// returns size of block or 0 on error:
bool get_block(StrView str, char delim, StrView* block) {
	char start_delim = delim;
	char end_delim;
	switch (delim) {
		case '(': end_delim = ')'; break;
		case '[': end_delim = ']'; break;
		case '{': end_delim = '}'; break;
		case '<': end_delim = '>'; break;
		default: EXIT();
	}
	uint32_t start_delim_pos;
	if (!find_delim(str, start_delim, &start_delim_pos)) { return false; }
	block->data = str.data + start_delim_pos;

	uint32_t end_delim_pos;
	if (!find_delim(str, end_delim, &end_delim_pos)) { return false; }
	block->len = (end_delim_pos - start_delim_pos) + 1;
	return true;
}

// returns true if it could finish
bool expand(Generator* gen, double frame_time, uint64_t frame_start) {
	Str* src_str;
	Str* dest_str;
	if (gen->expanded_string == gen->str0) {
		src_str = gen->str0;
		dest_str = gen->str1;
	} else {
		src_str = gen->str1;
		dest_str = gen->str0;
	}

	dest_str->len = 0;

	// uint32_t src_index = gen->current_index;
	StrView src_view = Str_get_view(src_str);
	StrView_offset(&src_view, gen->current_index);
	
	if (gen->current_iteration == 0) {
		Str *replacement = maybe_replace(gen, 'S',	Str_get_view_cstr("{}"));
		if (replacement) { 
			if (!eval_and_append_replacement(gen, dest_str, replacement)) { 
				printf("replacement eval failed!\n");
			}
		}
		gen->expanded_string = dest_str;
		return true;
	}

	while (src_view.data < src_str->data + src_str->len) {
		// check time, return false if out of time

#ifdef timed
		if (time_limit_reached(frame_start, frame_time)) {
			gen->current_index = src_str->data - src_view.data;
			return false;
		}
#endif

		char symbol = *src_view.data;

		if (symbol == '[' || symbol == ']') {
			if (!Str_putc(dest_str, symbol)) { EXIT(); }
			if (!StrView_offset(&src_view, 1)) { EXIT(); }
		} else if (symbol == '{' || symbol == '}') {
			EXIT();
		} else if (symbol == ' ') {
			if (!StrView_offset(&src_view, 1)) { EXIT(); }
		} else {
			StrView arg_block = {};

			if (!get_block(src_view, '{', &arg_block)) { EXIT(); }

			Str *replacement = maybe_replace(gen, symbol,	arg_block);
			if (replacement) { 
				if (!eval_and_append_replacement(gen, dest_str, replacement)) { 
					printf("replacement eval failed!\n");
				}
			} else {
				if (!Str_putc(dest_str, symbol)) { EXIT(); }
				if (!Str_put_view(dest_str, arg_block)) { EXIT(); }
			}
			if (!StrView_offset(&src_view, arg_block.len + 1)) { EXIT(); }
		}
	}

	gen->current_index = 0;
	gen->expanded_string = dest_str;
	return true;
}

bool generate_timed(Generator* gen, double frame_time, uint64_t frame_start) {
	while (gen->current_iteration < gen->iterations) {
		if (expand(gen, frame_time, frame_start)) {
			gen->current_iteration++;
		} else {
			return false;
		}
	}
	return true;
}

/* --- buiding --- */
Builder *builder_new() {
	Builder *builder = calloc(1, sizeof(Builder));
	builder->nodes = DynArr_new();
	builder->construct = DynArr_new();
	builder->generator_id = 5;
	builder->segment_node_count = 1;
	return builder;
}

void builder_free(Builder *builder) {
	if (!builder) { EXIT(); }
	DynArr_free(&builder->nodes);
	DynArr_free(&builder->construct);
	free(builder);
}

void builder_add_node(Builder *builder, Vec2 node) {
	uint32_t node_id = DynArr_push_back(builder->nodes, node);
	if (node_id == UINT32_MAX) { EXIT(); }
	builder->build_state.node_ids_queue[builder->build_state.queue_head] = node_id;
	builder->build_state.queue_head =
		(builder->build_state.queue_head + 1) % builder->segment_node_count;
}

void reset_builder(Builder* builder) {
	builder->reset_needed = false;
	builder->current_index = 0;
	builder->stack_index = 0;
	builder->build_state = builder->start_state;
	DynArr_clear(builder->nodes);
	DynArr_clear(builder->construct);
	builder_add_node(builder, builder->build_state.pos);
}

void reset_builder_draw(Builder *builder) {
	builder->redraw_needed = false;
	builder->current_construct_index = 0;
}

uint32_t add_builder(LManager* manager) {
	Builder *builder = builder_new();
	uint32_t id = SPSet_push_back(manager->builders, builder);
	return id;
}

bool remove_builder(LManager* manager, uint32_t id) {
	Builder *builder = SPSet_get(manager->builders, id);
	builder_free(builder);
	return SPSet_remove(manager->builders, id);
}

void clear_builders(LManager* manager) {
	for (int i = 0; i < DS_LEN(manager->builders); i++) {
		builder_free(SPSet_at(manager->builders, i));
	}
	SPSet_clear(manager->builders);
}

// move direction_vector * len
void _move(BuilderState* state, const double len) {
	state->pos.x += len * cos(state->angle);
	state->pos.y += len * -sin(state->angle);
}

// rotate value * pi
void _turn(BuilderState* state, const double angle) {
	state->angle += angle * PI;
}

void _change_width(BuilderState* state, const double width) {
	state->width += width; 
}

void symbol_action(Builder* builder, char symbol, double value) {
	if (isalpha(symbol)) {
		// update state and push back node
		_move(&builder->build_state, value);

		if (!islower(symbol)) {
			// push node
			builder_add_node(builder, builder->build_state.pos);

			// if there are enough nodes, push segment
			if (DS_LEN(builder->nodes) >= builder->segment_node_count) {
				Segment seg = {};
				seg.node_count = builder->segment_node_count;
				uint32_t queue_tail = builder->build_state.queue_head;
				for (uint32_t i = 0; i < builder->segment_node_count; i++) {
					seg.node_ids[i] =
						builder->build_state.node_ids_queue[(queue_tail + i) % builder->segment_node_count];
				}
				DynArr_push_back(builder->construct, seg);
			}
		}
	}

	// turn turtle counter-clockwise
	else if (symbol == '-') {
    _turn(&builder->build_state, -value);
	}
	// turn turtle clockwise
	else if (symbol == '+') {
    _turn(&builder->build_state, value);
	}
	// TODO: this should either set or change the width
	else if (symbol == '^') {
		_change_width(&builder->build_state, -value);
	}
	else if (symbol == '&') {
		_change_width(&builder->build_state, value);
	}
	// color palette
	else if (symbol == '$') {
	}
	else if (symbol == '%') {
	}
	// push and pop turtle state
	else if (symbol == '[') {
		if (builder->stack_index >= 4096) { EXIT(); }
    builder->stack[builder->stack_index++] = builder->build_state;
	}
	else if (symbol == ']') {
		if (builder->stack_index <= 0) { EXIT(); }
		builder->build_state = builder->stack[--builder->stack_index];
	} else {
		EXIT();
	}
}

// symbols can be S{} or S depending on the type
bool build_timed(Builder* builder, double frame_time, uint64_t frame_start) {
	if (!builder->lstring_non_owning) { return true; }
	StrView local_view = Str_get_view(builder->lstring_non_owning);
	if (builder->current_index > 0) {
		if (!StrView_offset(&local_view, builder->current_index)) { EXIT(); }
	}

	char* view_end = local_view.data + local_view.len;

  while (local_view.data < view_end) {

#ifdef timed
		if (time_limit_reached(frame_start, frame_time)) {
			builder->current_index = builder->lstring_non_owning.data - local_view.data;
			return false;
		}
#endif

		char symbol = *local_view.data;
		if (local_view.data + 1 < view_end && *(local_view.data + 1) == '{') {
			StrView block = {};
			if (!get_block(local_view, '{', &block)) { EXIT(); };
			StrView value_view = {block.data + 1, block.len - 1};

			// convert block to double
			// char value_str[64];
			// memcpy(value_str, block.data + 1, block.len - 1);
			// value_str[63] = '\n';
			// char* end;
			double value = strtod(value_view.data, NULL);

			symbol_action(builder, symbol, value);
			if (!StrView_offset(&local_view, block.len + 1)) { EXIT(); }
		} else if (symbol == ' ') {
			if (!StrView_offset(&local_view, 1)) { EXIT(); }
		} else if (symbol == '[' || symbol == ']') {
				symbol_action(builder, symbol, 0);
				if (!StrView_offset(&local_view, 1)) { EXIT(); }
		} else {
			EXIT();
		}
	}

  // ---- expansion completed ----
  builder->current_index = 0;
  return true;
}

bool draw_timed(Renderer* renderer, Builder *builder, double frame_time, uint64_t frame_start) {
	// for (uint32_t i = 0; i < DS_LEN(builder->nodes); i++) {
	// 	Vec2* pos = DynArr_at(builder->nodes, i);
	// 	draw_rect(renderer, *pos, add_Vec2(*pos, (Vec2){5,5}), 0xFF00FFFF);
	// }
	
	for (uint32_t i = builder->current_construct_index; i < DS_LEN(builder->construct); i++) {
#ifdef timed
		if (time_limit_reached(frame_start, frame_time)) {
			builder->current_construct_index = i;
			return false;
		}
#endif
		Segment* seg = DynArr_at(builder->construct, i);

		if (seg->node_count == 1) {
			Vec2* pos = DynArr_at(builder->nodes, seg->node_ids[0]);
			draw_rect(renderer, *pos, add_Vec2(*pos, (Vec2){5,5}), 0xFF00FFFF);
		}
		else if (seg->node_count == 2) {
			Vec2* pos0 = DynArr_at(builder->nodes, seg->node_ids[0]);
			Vec2* pos1 = DynArr_at(builder->nodes, seg->node_ids[1]);
			draw_thick_line(renderer, *pos0,*pos1,
											20.0, 0xFF00FFFF);
		}
		else if (seg->node_count == 3) {
			Vec2* pos0 = DynArr_at(builder->nodes, seg->node_ids[0]);
			Vec2* pos1 = DynArr_at(builder->nodes, seg->node_ids[1]);
			Vec2* pos2 = DynArr_at(builder->nodes, seg->node_ids[2]);
			draw_trigon(renderer, *pos0, *pos1, *pos2, 0xFF00FFFF);
		}
		else {
			EXIT();
		}
	}

	builder->current_construct_index = 0;
	return true;
}

bool update_lsystem(Renderer *renderer, LManager *manager, double frame_time, uint64_t frame_start) {
	bool out_of_time = false;
	for (size_t i = 0; i < DS_LEN(manager->generators); i++) {
		Generator* generator = SPSet_at(manager->generators, i);
		uint32_t generator_id = SPSet_id_at(manager->generators, i);

		switch (generator->state) {
			case IDLE:
				if (!generator->reset_needed) {
					break;
				}
				reset_generator(generator); // reset_needed == false
				generator->state = WORKING;
			case WORKING:
				if (generator->reset_needed) {
					reset_generator(generator);
				}

				if (!generate_timed(generator, frame_time, frame_start)) {
					out_of_time = true;
					break;
				}
				Str_print(generator->expanded_string);

				// mark all registered interpreters for reset
				for (size_t i = 0; i < DS_LEN(manager->builders); i++) {
					Builder *builder = SPSet_at(manager->builders, i);
					printf("builder->generator_id: %d\n", builder->generator_id);
					printf("generator_id: %d\n", generator_id);
					if (builder->generator_id == generator_id) {
						builder->lstring_non_owning = generator->expanded_string;
						builder->reset_needed = true;
					}
				}
				generator->state = IDLE;
				break;
			default: EXIT();
		}
	}

	for (size_t i = 0; i < DS_LEN(manager->builders); i++) {
		Builder *builder = SPSet_at(manager->builders, i);
		printf("builder nodes: %d\n", DS_LEN(builder->construct));

		if (!out_of_time) {
			switch(builder->state) {
				case IDLE:
					if (!builder->reset_needed) {
						break;
					}
					reset_builder(builder); // inter_reset_needed = false
					builder->state = WORKING;	
				case WORKING:
					if (builder->reset_needed) {
						reset_builder(builder);
					}

					if (!build_timed(builder, frame_time, frame_start)) {
						out_of_time = true;
						break;
					}
					
					redraw_all(manager);
					builder->state = IDLE;

					break;
				default: EXIT();
			}
		}

		// draw constructs into buffer
		if (!out_of_time) {
			// need to redraw_all objects if i zoom or pan or change pos
			switch (builder->draw_state) {
				case IDLE: 
					if (!builder->redraw_needed) {
						break;
					}
					reset_builder_draw(builder);
					builder->draw_state = WORKING;	
				case WORKING:

					if (builder->redraw_needed) {
						reset_builder_draw(builder);
					}

					if (!draw_timed(renderer, builder, frame_time, frame_start)) {
						out_of_time = true;
						break;
					}

					builder->draw_state = IDLE;
					break;
				default: EXIT();
			}
		}
	}
	return out_of_time;
}
