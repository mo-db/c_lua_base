#include "lang_gen.h"

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

// simpler if i had arena
void delete_generator(Generator* generator) {
	SSet_free(&generator->vars);
	SSet_free(&generator->productions);
	free(generator);
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

// i could use str views instead of memory, just one string and views
Production parse_production_str(char* str_in) {
	if (!str_in) { EXIT(); }

	Production prod = {};
	prod.str = LS_new_from_cstring(str_in);
	LSView str_view = LS_get_view(prod.str);

	uint32_t delim_pos = 0;
	if (!find_delim(str_view, '!', &delim_pos)) { EXIT(); }
	prod.replacement.data = str_view.data + delim_pos + 1;
	prod.replacement.len = str_view.len - (delim_pos + 1);


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

void update_generator(Generator* gen) {
	if (gen->reset_needed) {
		reset_generator(gen);
		gen->reset_needed = false;
	}
	
	if (!gen->done_generating) {
		if (generate_timed(gen)) {
			gen->done_generating = true;


			LS_print(gen->expanded_string);
			printf("len: %d\nnew lstring:\n", gen->expanded_string.len);
			printf("current_iteration: %d\n", gen->current_iteration);
			printf("\n");
		}
	}
}
