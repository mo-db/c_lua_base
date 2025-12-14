#include "lang_gen.h"

// --- manager ---
LManager LManager_new() {
	LManager manager = {};
	SSet_alloc(&manager.generators, OBJ_MAX);
	SSet_alloc(&manager.builders, OBJ_MAX);
	return manager;
}
void LManager_delete(LManager* manager) {
	SSet_free(&manager->generators);
	SSet_free(&manager->builders);
}

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
	} else { EXIT(); }

	if (lua_table_get_number(L, "rotate", &value)) {
		generator->rotate_default = value;
	} else { EXIT(); }
}

void production_free(Production* prod) {
	Str_free(prod->str);
}

static void update_global_value(Generator *generator, double value,
                         const char ch) {
  double *gen_value =
      generator_get_var_value(&generator->vars, generator->char_id_map, ch);
	// if character is not mapped in vars add it
  if (!gen_value) {
    if (generator_add_var(&generator->vars, generator->char_id_map, value,
                          ch)) {
    } else {
      EXIT();
    }
  }
	// else update it
  else {
    *gen_value = value;
  }
}

// name shoud be maybe set or similar
static void configure_globals(lua_State *L, Generator *generator) {
  double value = 0;

  if (lua_table_get_number(L, "h", &value)) {
		update_global_value(generator, value, 'h');
	}
  if (lua_table_get_number(L, "i", &value)) {
		update_global_value(generator, value, 'i');
	}
  if (lua_table_get_number(L, "j", &value)) {
		update_global_value(generator, value, 'j');
	}
  if (lua_table_get_number(L, "k", &value)) {
		update_global_value(generator, value, 'k');
	}
}

static void configure_productions(lua_State *L, Generator *generator) {

	// TODO: put prod into SS, remove should call free_prod
	// clear productions
	for (int i = 0; i < SSET_LEN(generator->productions); i++) {
		production_free(SSet_at(&generator->productions, i));
	}
	SSet_clear(&generator->productions);

	for (int i = 1;; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		if (lua_isstring(L, -1)) {
			Production prod = parse_production_str(generator, lua_tolstring(L, -1, NULL));

			LSView blub = LS_get_view(prod.str);
			printf("### PROD DEBUG ###: %d\n", i);
			printf("###\n");
			LSView_trim(&blub);
			LS_print(blub);
			LSView_trim(&prod.symbol);
			LS_print(prod.symbol);
			LSView_trim(&prod.condition);
			LS_print(prod.condition);
			LSView_trim(&prod.context);
			LS_print(prod.context);
			LSView_trim(&prod.replacement);
			LS_print(prod.replacement);
			printf("sset len: %d\n", SSET_LEN(generator->productions));
			printf("###\n");


			if (SSet_push_back(&generator->productions, &prod) == UINT32_MAX) { EXIT(); }
		} else {
			break;
		}
		lua_pop(L, 1);
	}
}

void reconfigure_system(lua_State *L, LManager *manager) {
	lua_getglobal(L, "generator_configs");
	if (!lua_istable(L, -1)) { EXIT(); }
	for (size_t i = 0; i < SSET_LEN(manager->generators); i++) {
		Generator *generator = SSet_at(&manager->generators, i);
		uint32_t generator_id = SSet_id_at(&manager->generators, i);
		generator->reset_needed = true;

		lua_pushnumber(L, generator_id);
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			lua_pushstring(L, "default");
			lua_gettable(L, -2);
			if (!lua_istable(L, -1)) { EXIT(); }
		}
		// generator_config found

		// get field defaults
		lua_pushstring(L, "defaults");
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) { EXIT(); }

		configure_defaults(L, generator);
		lua_pop(L, 1);

		// get field globals
		lua_pushstring(L, "globals");
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) { EXIT(); }

		configure_globals(L, generator);
		lua_pop(L, 1);

		// get field productions
		lua_pushstring(L, "productions");
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) { EXIT(); }

		configure_productions(L, generator);

		// for (int i = 0; i < SSET_LEN(generator->productions); i++) {
		// 	Production* prod = SSet_at(&generator->productions, i);
		// 	LSView blub = LS_get_view(prod->str);
		// 	printf("### PROD DEBUG ###: %d\n", i);
		// 	printf("###\n");
		// 	LSView_trim(&blub);
		// 	LS_print(blub);
		// 	LSView_trim(&prod->symbol);
		// 	LS_print(prod->symbol);
		// 	LSView_trim(&prod->condition);
		// 	LS_print(prod->condition);
		// 	LSView_trim(&prod->context);
		// 	LS_print(prod->context);
		// 	LSView_trim(&prod->replacement);
		// 	LS_print(prod->replacement);
		// 	printf("###\n");
		// }

		lua_pop(L, 1);

		// pop gerneator_config table
		lua_pop(L, 1);
	}

	double value = 0;

	lua_getglobal(L, "builder_configs");
	if (!lua_istable(L, -1)) { EXIT(); }
	for (size_t i = 0; i < SSET_LEN(manager->builders); i++) {
		Builder *builder = SSet_at(&manager->builders, i);
		uint32_t builder_id = SSet_id_at(&manager->builders, i);
		builder->reset_needed = true;

		lua_pushnumber(L, builder_id);
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			lua_pushstring(L, "default");
			lua_gettable(L, -2);
			if (!lua_istable(L, -1)) { EXIT(); }
		}
		// builder_config found
		// get field pos
		lua_pushstring(L, "pos");
		lua_gettable(L, -2);
		if (!lua_istable(L, -1)) { EXIT(); }
		if (!lua_table_number_at(L, 1, &value)) { EXIT(); }
		builder->start_state.pos.x = value;
		if (!lua_table_number_at(L, 2, &value)) { EXIT(); }
		builder->start_state.pos.y = value;
		lua_pop(L, 1);

		// get all other fields
		if (!lua_table_get_number(L, "angle", &value)) { EXIT(); }
		builder->start_state.angle = value;
		if (!lua_table_get_number(L, "segment_node_count", &value)) { EXIT(); }
		builder->segment_node_count = value;
		if (!lua_table_get_number(L, "generator_id", &value)) { EXIT(); }
		builder->generator_id = value;

		// pop gerneator_config table
		lua_pop(L, 1);
	}
}

bool generator_add_var(SSet_double* sset, uint32_t* map, double value, char ch) {
	uint32_t id = SSet_push_back(sset, &value);
	if (id == UINT32_MAX) {
		return false;
	}
	map[(uint32_t)ch] = id;
	return true;
}

double* generator_get_var_value(SSet_double* sset, uint32_t* map, char ch) {
	uint32_t var_id = map[(uint32_t)ch];
	double* value = NULL;
	if (var_id != UINT32_MAX) {
		value = SSet_get(sset, var_id);
	}
	if (!value) {
		return NULL;
	}
	return value;
}

void redraw_all(LManager *manager) {
	for (size_t i = 0; i < SSET_LEN(manager->builders); i++) {
		Builder *builder = SSet_at(&manager->builders, i);
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

// --- c functions that get called by lua ---
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


// --- generation ---
Generator new_generator() {
	Generator gen = {};
	SSet_alloc(&gen.vars, 128);
	SSet_alloc(&gen.productions, 64);
	gen.replacement_buffer = LS_new(4096);
	gen.str0 = LS_new(100000000);
	gen.str1 = LS_new(100000000);
	gen.expanded_string = LS_get_view(gen.str0);
	gen.iterations = 1;
	for (int i = 0; i < 128; i++) {
		gen.char_id_map[i] = UINT32_MAX;
	}
	return gen;
}

void delete_generator(Generator gen) {
	SSet_free(&gen.vars);
	SSet_free(&gen.productions);
	LS_free(gen.replacement_buffer);
	LS_free(gen.str0);
	LS_free(gen.str1);
}

void reset_generator(Generator* gen) {
		gen->current_iteration = 0;
		gen->current_index = 0;
		LS_clear(&gen->replacement_buffer);
		LS_clear(&gen->str0);
		LS_clear(&gen->str1);
		gen->expanded_string = LS_get_view(gen->str0);
		gen->reset_needed = false;
}

uint32_t add_generator(LManager* manager) {
	Generator gen = new_generator();
	uint32_t id = SSet_push_back(&manager->generators, &gen);
	return id;
}

bool remove_generator(LManager* manager, uint32_t generator_id) {
	return SSet_remove(&manager->generators, generator_id);
}

void clear_generators(LManager* manager) {
	SSet_clear(&manager->generators);
}

bool arg_block_empty(LSView arg_block) {
	if (*arg_block.data != '{') { EXIT(); }

	while (*(++arg_block.data) != '}')
		if (*arg_block.data != ' ') {
			return false;
		}
	return true;
}

// find delim in LS
bool find_delim(LSView str, char delim, uint32_t* pos) {
	for (uint32_t i = 0; i < str.len; i++) {
		if (str.data[i] == delim) {
			*pos = i;
			return true;
		}
	}
	*pos = str.len;
	return false;
}

LS cut_args(const LSView view) {
	char* ch = view.data;
	char* ls_end = view.data + view.len;
	LS new_ls = LS_new(view.len);
	while (ch < ls_end) {
		if (*ch != '{') {
			LS_append_char(&new_ls, *ch);
			ch++;
		} else {
			while (ch < ls_end && *ch != '}') { ch++; }
			ch++;
		}
	}
	return new_ls;
}

// i could use str views instead of memory, just one string and views
Production parse_production_str(Generator* gen, const char* str) {
	if (!str) { EXIT(); }

	// TODO: i need a new_prod() function instead of doing this here!

	// --- first split ---
	uint32_t delim_pos = 0;
	if (!find_delim(get_view(str), '!', &delim_pos)) { EXIT(); }

	// --- format replacement ---
	LSView first_part_view = get_view(str);
	first_part_view.len = delim_pos;
	LS repl = LS_new(4096);
	LSView repl_view = get_view(str + delim_pos + 1);

	char* prepl_end = repl_view.data + repl_view.len;
	while (repl_view.data < prepl_end) {
		char symbol = *repl_view.data;

		if (symbol == ' ') {
			if (!LSView_offset(&repl_view, 1)) { EXIT(); }
			continue;
		}

		if (symbol == '[' || symbol == ']') {
			if (!LS_append_char(&repl, symbol)) { EXIT(); }
			if (!LSView_offset(&repl_view, 1)) { EXIT(); }
			continue;
		}

		if (repl_view.data + 1 < prepl_end) {
			SymbolCategory symbol_category = get_symbol_category(symbol);
			if (!LS_append_char(&repl, symbol)) { EXIT(); }
			if (!LSView_offset(&repl_view, 1)) { EXIT(); }

			if (*repl_view.data != '{') {
				double default_value = get_default(gen, symbol_category);
				char new_arg_block[64];
				new_arg_block[63] = '\0';
				snprintf(new_arg_block, 64, "{%f}", default_value);
				if (LS_append(&repl, get_view(new_arg_block)) == 0) { EXIT(); }
			} else {
				LSView block = {};
				if (!get_block(repl_view, '{', &block)) { EXIT(); };
				if (arg_block_empty(block)) {
					double default_value = get_default(gen, symbol_category);
					char new_arg_block[64];
					new_arg_block[63] = '\0';
					snprintf(new_arg_block, 64, "{%f}", default_value);
					if (LS_append(&repl, get_view(new_arg_block)) == 0) { EXIT(); }
				} else {
					if (LS_append(&repl, block) == 0) { EXIT(); }
				}
				if (!LSView_offset(&repl_view, block.len)) { EXIT(); }
			}
		}
	}



	Production prod = {};
	prod.str.data = malloc(first_part_view.len + repl.len + 2);
	prod.str.cap = first_part_view.len + repl.len + 2;
	prod.str.len = 0;
	prod.replacement = LS_get_view(prod.str);
	prod.condition = LS_get_view(prod.str);
	prod.context = LS_get_view(prod.str);
	prod.symbol = LS_get_view(prod.str);
	if (!LS_append(&prod.str, first_part_view)) { EXIT(); }
	if (!LS_append_char(&prod.str, '!')) { EXIT(); }
	if (!LS_append(&prod.str, LS_get_view(repl))) { EXIT(); }

	prod.replacement.data = prod.str.data + first_part_view.len + 1;
	prod.replacement.len = repl.len;

	LS_free(repl);

	// --- split all other parts ---
	LSView str_view = LS_get_view(prod.str);

	str_view.len = first_part_view.len;
	if (!find_delim(str_view, ':', &delim_pos)) { 
		prod.symbol = str_view;
		return prod;
	}
	prod.symbol.data = str_view.data;
	prod.symbol.len = delim_pos;


	if (!LSView_offset(&str_view, delim_pos + 1)) { EXIT(); }
	if (!find_delim(str_view, ':', &delim_pos)) { 
		prod.condition = str_view;
		return prod;
	}
	prod.condition.data = str_view.data;
	prod.condition.len = delim_pos;


	if (!LSView_offset(&str_view, delim_pos + 1)) { EXIT(); }
	prod.context = str_view;


	return prod;
}

// evaluate production into replacement cache
// replacement has been formated to:
// [S{x,y,z} or not ? do i need defaults, i think i do, scaling vals
// first try without args
bool eval_production(Generator* gen, LSView replacement) {
	// skip for testing
	gen->replacement_buffer.len = 0;
	if (LS_append(&gen->replacement_buffer, replacement) == 0) {
		EXIT();
	}
	return true;

	// complex later
	// int new_prod_index = 0;	
	int prod_index = 0;	
	// bool result = true;
	while (prod_index < replacement.len) {
		//result = get_arg_block();	

		// parse into strings
		//result = parse_arg_block();

		// arraylist of n doubles 
		//result = eval_args();

		// combine into evaluated arg block string
		//snprintf()
		// copy onto prod_cache
	}
}

// give pobuilder to symbole
LSView* maybe_get_replacement(Generator* gen, char symbol, LSView block) {
	if (block.len <= 2) {
		// no args
	}
	for (uint32_t i = 0; i < SSET_LEN(gen->productions); i++) {
		Production* prod = SSet_at(&gen->productions, i);
		if (*prod->symbol.data != symbol) { continue; }

		// check condition

		// check context

		return &prod->replacement;
	}
	return NULL;
}

// get pobuilder to start and end of {} block
// returns size of block or 0 on error:
bool get_block(LSView str, char delim, LSView* block) {
	char start_delim = delim;
	char end_delim;
	switch (delim) {
		case '(': end_delim = ')'; break;
		case '[': end_delim = ']'; break;
		case '{': end_delim = '}'; break;
		case '<': end_delim = '>'; break;
		default: EXIT_MSG("delim is no valid bracket type");
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
	LS* src_str;
	LS* dest_str;
	if (gen->expanded_string.data == gen->str0.data) {
		src_str = &gen->str0;
		dest_str = &gen->str1;
	} else {
		src_str = &gen->str1;
		dest_str = &gen->str0;
	}

	dest_str->len = 0;

	// uint32_t src_index = gen->current_index;
	LSView src_view = LS_get_view(*src_str);
	LSView_offset(&src_view, gen->current_index);
	
	if (gen->current_iteration == 0) {
		LSView* replacement = maybe_get_replacement(gen, 'S',	get_view("{}"));
		if (!replacement) { EXIT(); }
		if (replacement->len == 0) { EXIT(); }
		if (!eval_production(gen, *replacement)) { EXIT(); }
		if (LS_append(dest_str, LS_get_view(gen->replacement_buffer)) == 0) { EXIT(); }
		gen->expanded_string = LS_get_view(*dest_str);
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
			if (!LS_append_char(dest_str, symbol)) { EXIT(); }
			if (!LSView_offset(&src_view, 1)) { EXIT(); }
		} else if (symbol == '{' || symbol == '}') {
			EXIT();
		} else if (symbol == ' ') {
			if (!LSView_offset(&src_view, 1)) { EXIT(); }
		} else {
			LSView arg_block = {};

			if (!get_block(src_view, '{', &arg_block)) { EXIT(); }
			LSView* replacement = maybe_get_replacement(gen, symbol, arg_block);
			if (replacement) {
				if (!eval_production(gen, *replacement)) { EXIT(); }
				if (LS_append(dest_str, LS_get_view(gen->replacement_buffer)) == 0) {
					EXIT();
				}
			} else {
				if (!LS_append_char(dest_str, symbol)) { EXIT(); }
				if (LS_append(dest_str, arg_block) == 0) { EXIT(); }
			}
			if (!LSView_offset(&src_view, arg_block.len + 1)) { EXIT(); }
		}
	}

	gen->current_index = 0;
	gen->expanded_string = LS_get_view(*dest_str);
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

// --- generation ---
Builder new_builder() {
	Builder builder = {};
	SSet_alloc(&builder.nodes, 1 << 24);
	SSet_alloc(&builder.construct, 1 << 24);
	builder.generator_id = OBJ_MAX; // cant be reached
	return builder;
}

void delete_builder(Builder builder) {
	SSet_free(&builder.nodes);
	SSet_free(&builder.construct);
}

void builder_add_node(Builder *builder, Vec2 *node) {
	uint32_t node_id = SSet_push_back(&builder->nodes, node);
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
	SSet_clear(&builder->nodes);
	SSet_clear(&builder->construct);
	builder_add_node(builder, &builder->build_state.pos);
}

void reset_builder_draw(Builder *builder) {
	builder->redraw_needed = false;
	builder->current_construct_index = 0;
}

uint32_t add_builder(LManager* manager) {
	Builder builder = new_builder();
	uint32_t id = SSet_push_back(&manager->builders, &builder);
	return id;
}

bool remove_builder(LManager* manager, uint32_t builder_id) {
	return SSet_remove(&manager->builders, builder_id);
}

void clear_builders(LManager* manager) {
	SSet_clear(&manager->builders);
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
			builder_add_node(builder, &builder->build_state.pos);

			// if there are enough nodes, push segment
			if (SSET_LEN(builder->nodes) >= builder->segment_node_count) {
				Segment seg = {};
				seg.node_count = builder->segment_node_count;
				uint32_t queue_tail = builder->build_state.queue_head;
				for (uint32_t i = 0; i < builder->segment_node_count; i++) {
					seg.node_ids[i] =
						builder->build_state.node_ids_queue[(queue_tail + i) % builder->segment_node_count];
				}
				SSet_push_back(&builder->construct, &seg);
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


// ---- building ----
bool build_timed(Builder* builder, double frame_time, uint64_t frame_start) {

	// printf("Local VIEW\n");
	// LS_print(local_view);
	LSView local_view = builder->view;
	if (builder->current_index > 0) {
		if (!LSView_offset(&local_view, builder->current_index)) { EXIT(); }
	}

	char* view_end = builder->view.data + builder->view.len;

  while (local_view.data < view_end) {

    // ---- check time ----

#ifdef timed
		if (time_limit_reached(frame_start, frame_time)) {
			builder->current_index = builder->view.data - local_view.data;
			return false;
		}
#endif
		char symbol = *local_view.data;
		
		if (local_view.data + 1 < view_end && *(local_view.data + 1) == '{') {
			LSView block = {};
			if (!get_block(local_view, '{', &block)) { EXIT(); };

			// convert block to double
			char value_str[64];
			memcpy(value_str, block.data + 1, block.len - 1);
			value_str[63] = '\n';
			char* end;
			double value = strtod(value_str, &end);

			symbol_action(builder, symbol, value);
			if (!LSView_offset(&local_view, block.len + 1)) { EXIT(); }
		} else if (symbol == ' ') {
			if (!LSView_offset(&local_view, 1)) { EXIT(); }
		} else if (symbol == '[' || symbol == ']') {
				symbol_action(builder, symbol, 0);
				if (!LSView_offset(&local_view, 1)) { EXIT(); }
		} else {
			EXIT();
		}
	}

  // ---- expansion completed ----
  builder->current_index = 0;
  return true;
}

bool draw_timed(Renderer* renderer, Builder *builder, double frame_time, uint64_t frame_start) {
	// for (uint32_t i = 0; i < SSET_LEN(builder->nodes); i++) {
	// 	Vec2* pos = SSet_at(&builder->nodes, i);
	// 	draw_rect(renderer, *pos, add_Vec2(*pos, (Vec2){5,5}), 0xFF00FFFF);
	// }
	
	for (uint32_t i = builder->current_construct_index; i < SSET_LEN(builder->construct); i++) {
#ifdef timed
		if (time_limit_reached(frame_start, frame_time)) {
			builder->current_construct_index = i;
			return false;
		}
#endif
		Segment* seg = SSet_at(&builder->construct, i);

		if (seg->node_count == 1) {
			Vec2* pos = SSet_at(&builder->nodes, seg->node_ids[0]);
			draw_rect(renderer, *pos, add_Vec2(*pos, (Vec2){5,5}), 0xFF00FFFF);
		}
		else if (seg->node_count == 2) {
			Vec2* pos0 = SSet_at(&builder->nodes, seg->node_ids[0]);
			Vec2* pos1 = SSet_at(&builder->nodes, seg->node_ids[1]);
			draw_thick_line(renderer, *pos0,*pos1,
											20.0, 0xFF00FFFF);
		}
		else if (seg->node_count == 3) {
			Vec2* pos0 = SSet_at(&builder->nodes, seg->node_ids[0]);
			Vec2* pos1 = SSet_at(&builder->nodes, seg->node_ids[1]);
			Vec2* pos2 = SSet_at(&builder->nodes, seg->node_ids[2]);
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
	for (size_t i = 0; i < SSET_LEN(manager->generators); i++) {
		Generator* generator = SSet_at(&manager->generators, i);
		uint32_t generator_id = SSet_id_at(&manager->generators, i);

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

				uint64_t now = SDL_GetPerformanceCounter();
				if (!generate_timed(generator, frame_time, frame_start)) {
					out_of_time = true;
					break;
				}
				double elapsed =
					((double)(SDL_GetPerformanceCounter() - now) / SDL_GetPerformanceFrequency()) * 1000;
				 printf("gen-time: %f\n", elapsed);
				LS_print(generator->expanded_string);

				// mark all registered interpreters for reset
				for (size_t i = 0; i < SSET_LEN(manager->builders); i++) {
					Builder *builder = SSet_at(&manager->builders, i);
					if (builder->generator_id == generator_id) {
						builder->view = generator->expanded_string;
						builder->reset_needed = true;
					}
				}
				generator->state = IDLE;
				break;
			default: EXIT();
		}
	}



	for (size_t i = 0; i < SSET_LEN(manager->builders); i++) {
		Builder *builder = SSet_at(&manager->builders, i);



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

  				uint64_t now = SDL_GetPerformanceCounter();
					if (!build_timed(builder, frame_time, frame_start)) {
						out_of_time = true;
						break;
					}

					double elapsed =
      			((double)(SDL_GetPerformanceCounter() - now) / SDL_GetPerformanceFrequency()) * 1000;
					 printf("build-time: %f\n", elapsed);

					
					// renderer_clear(&renderer->pixelbuffer, 0xFF000000); // not here!!
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

					uint64_t now = SDL_GetPerformanceCounter();

					if (!draw_timed(renderer, builder, frame_time, frame_start)) {
						out_of_time = true;
						break;
					}

					double elapsed =
						((double)(SDL_GetPerformanceCounter() - now) / SDL_GetPerformanceFrequency()) * 1000;
					 printf("draw-time: %f\n", elapsed);

					builder->draw_state = IDLE;
					break;
				default: EXIT();
			}
		}




	}



	return out_of_time;


}


