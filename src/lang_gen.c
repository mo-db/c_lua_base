#include "lang_gen.h"
// blub
bool arg_block_empty(LSView arg_block) {
	if (*arg_block.data != '{') { EXIT(); }

	while (*(++arg_block.data) != '}')
		if (*arg_block.data != ' ') {
			return false;
		}
	return true;
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

// gen
Generator new_generator() {
	Generator gen = {};
	SSet_alloc(&gen.vars, 128);
	SSet_alloc(&gen.productions, 64);
	gen.replacement_buffer = LS_new(4096);
	gen.str0 = LS_new(100000000);
	gen.str1 = LS_new(100000000);
	gen.expanded_string = LS_get_view(gen.str0);
	gen.iterations = 1;
	gen.current_index = 0;
	return gen;
}

void delete_generator(Generator gen) {
	SSet_free(&gen.vars);
	SSet_free(&gen.productions);
	LS_free(gen.replacement_buffer);
	LS_free(gen.str0);
	LS_free(gen.str1);
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
Production parse_production_str(Generator* gen, char* str) {
	if (!str) { EXIT(); }

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
			if (!LS_append_char(&repl, symbol)) { EXIT(); }
			if (!LSView_offset(&repl_view, 1)) { EXIT(); }

			if (*repl_view.data != '{') {
				double default_value = get_default(gen, get_symbol_category(symbol));
				char new_arg_block[64];
				new_arg_block[63] = '\0';
				snprintf(new_arg_block, 64, "{%f}", default_value);
				if (LS_append(&repl, get_view(new_arg_block)) == 0) { EXIT(); }
			} else {
				LSView block = {};
				if (!get_block(repl_view, '{', &block)) { EXIT(); };
				printf("block:\n");
				LS_print(block);
				printf("empty: %d\n", arg_block_empty(block));
				if (arg_block_empty(block)) {
					double default_value = get_default(gen, get_symbol_category(symbol));
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
	prod.str = LS_new(first_part_view.len + repl.len + 5);
	if (!LS_append(&prod.str, first_part_view)) { EXIT(); }
	if (!LS_append_char(&prod.str, '!')) { EXIT(); }
	if (!LS_append(&prod.str, LS_get_view(repl))) { EXIT(); }

	prod.replacement.data = prod.str.data + first_part_view.len + 1;
	prod.replacement.len = repl.len;

	LS_free(repl);

	// --- split all other parts ---
	LSView str_view = LS_get_view(prod.str);
	str_view.len = delim_pos;
	if (!find_delim(str_view, ':', &delim_pos)) { 
		prod.symbol = str_view;
		return prod;
	}
	prod.symbol.data = str_view.data;
	prod.symbol.len = delim_pos;

	str_view.len -= (delim_pos + 1);
	str_view.data += delim_pos + 1;
	if (!find_delim(str_view, ':', &delim_pos)) { 
		prod.condition = str_view;
		return prod;
	}
	prod.condition.data = str_view.data;
	prod.condition.len = delim_pos;

	str_view.len -= (delim_pos + 1);
	str_view.data += delim_pos + 1;
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

// give pointer to symbole
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

// get pointer to start and end of {} block
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
bool expand(Generator* gen) {


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
	src_view.data += gen->current_index;
	src_view.len -= gen->current_index;

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

		char symbol = *src_view.data;

		if (symbol == '[' || symbol == ']') {
			if (!LS_append_char(dest_str, symbol)) { EXIT(); }
			src_view.data++;
			src_view.len--;
		} else if (symbol == '{' || symbol == '}') {
			EXIT();
		} else if (symbol == ' ') {
			src_view.data++;
			src_view.len--;
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
			src_view.data += arg_block.len + 1;
			src_view.len -= (arg_block.len + 1);
		}
	}

	gen->expanded_string = LS_get_view(*dest_str);
	return true;
}

bool generate_timed(Generator* gen) {
	while (gen->current_iteration < gen->iterations) {
		if (expand(gen)) {
			gen->current_iteration++;
		} else {
			return false;
		}
	}
	return true;
}

void reset_generator(Generator* gen) {
		gen->current_iteration = 0;
		gen->current_index = 0;
		gen->done_generating = false;
}

bool update_generator(Generator* gen) {
	if (gen->reset_needed) {
		reset_generator(gen);
		gen->reset_needed = false;
	}
	
	if (!gen->done_generating) {
		if (generate_timed(gen)) {
			gen->done_generating = true;

			LS_print(gen->expanded_string);
			printf("Without args:\n");
			LS without_args = cut_args(gen->expanded_string);
			LS_print(LS_get_view(without_args));
			LS_free(without_args);
			printf("len: %d\nnew lstring:\n", gen->expanded_string.len);
			printf("current_iteration: %d\n", gen->current_iteration);
			printf("\n");
			return true;
		}
	}
	return false;
}


// --- generation ---
Interpreter new_interpreter(LSView view, InterpreterState state) {
	Interpreter inter = {};
	SSet_alloc(&inter.nodes, 1 << 24);
	SSet_alloc(&inter.segments, 1 << 24);
	inter.view = view;
	inter.state = state;
	inter.start_state = state;
	inter.done_building = false;
	inter.done_redraw = false;
	return inter;
}

void delete_interpreter(Interpreter inter) {
	SSet_free(&inter.nodes);
	SSet_free(&inter.segments);
}

// move direction_vector * len
void _move(InterpreterState* state, const double len) {
	state->pos.x += len * cos(state->angle);
	state->pos.y += len * -sin(state->angle);
}

// rotate value * pi
void _turn(InterpreterState* state, const double angle) {
	state->angle += angle * PI;
}

void _change_width(InterpreterState* state, const double width) {
	state->width += width; 
}

void symbol_action(Interpreter* inter, char symbol, double value) {
	if (isalpha(symbol)) {
		// update state and push back node
		_move(&inter->state, value);

		if (!islower(symbol)) {
			// push node
			uint32_t node_id = SSet_push_back(&inter->nodes, &inter->state.pos);
			// push node_id to node_ids_queue
			inter->state.node_ids_queue[inter->state.queue_head] = node_id;
			inter->state.queue_head = (inter->state.queue_head + 1) % SEG_MAX_NODES;
			// if there are enough nodes, push segment
			if (SSET_LEN(inter->nodes) >= SEG_MAX_NODES) {
				Segment seg = {};
				uint32_t queue_tail = inter->state.queue_head + 1;
				for (int i = 0; i < SEG_MAX_NODES; i++) {
					seg.node_ids[i] = inter->state.node_ids_queue[(queue_tail + i) % SEG_MAX_NODES];
				}
				SSet_push_back(&inter->segments, &seg);
			}
		}
	}

	// turn turtle counter-clockwise
	else if (symbol == '-') {
    _turn(&inter->state, -value);
	}
	// turn turtle clockwise
	else if (symbol == '+') {
    _turn(&inter->state, value);
	}
	// TODO: this should either set or change the width
	else if (symbol == '^') {
		_change_width(&inter->state, -value);
	}
	else if (symbol == '&') {
		_change_width(&inter->state, value);
	}
	// color palette
	else if (symbol == '$') {
	}
	else if (symbol == '%') {
	}
	// push and pop turtle state
	else if (symbol == '[') {
		if (inter->stack_index >= 4096) { EXIT(); }
    inter->stack[inter->stack_index++] = inter->state;
	}
	else if (symbol == ']') {
		if (inter->stack_index <= 0) { EXIT(); }
		inter->state = inter->stack[--inter->stack_index];
	} else {
		EXIT();
	}
}


// ---- building ----
bool build_timed(Interpreter* inter) {

	LSView local_view = inter->view;
	if (inter->current_index > 0) {
		LSView_offset(&local_view, inter->current_index);
	}


	printf("here!\n");
	LS_print(local_view);

	char* view_end = inter->view.data + inter->view.len;

  while (local_view.data < view_end) {

    // ---- check time ----

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

			printf("symbol action value: %f\n", value);

			symbol_action(inter, symbol, value);
			if (!LSView_offset(&local_view, block.len + 1)) { EXIT(); }
		} else if (symbol == ' ') {
			if (!LSView_offset(&local_view, 1)) { EXIT(); }
		} else if (symbol == '[' || symbol == ']') {
				symbol_action(inter, symbol, 0);
				if (!LSView_offset(&local_view, 1)) { EXIT(); }
		} else {
			EXIT();
		}
	}

  // ---- expansion completed ----
  inter->current_index = 0;
  return true;
}



void reset_inter(Interpreter* inter) {
		inter->current_index = 0;
		inter->done_building = false;
}

bool update_inter(Interpreter* inter) {
	if (inter->reset_needed) {
		reset_inter(inter);
		inter->reset_needed = false;
	}

	
	if (!inter->done_building) {
		if (build_timed(inter)) {
			inter->done_building = true;
			return true;
		}
	}
	return false;
}
