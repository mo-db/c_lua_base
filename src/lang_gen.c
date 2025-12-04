#include "lang_gen.h"

Generator new_generator() {
	Generator gen = {};
	SSet_alloc(&gen.vars, 128);
	SSet_alloc(&gen.productions, 64);
	gen.old_string.data = gen.old_string_cache;
	gen.new_string.data = gen.new_string_cache;
	gen.replace.data = gen.replacement_cache;
	gen.iterations = 1;
	return gen;
}

// simpler if i had arena
void delete_generator(Generator* generator) {
	SSet_free(&generator->vars);
	SSet_free(&generator->productions);
	free(generator);
}

// find delim in LS
bool find_delim(LS str, char delim, uint32_t* pos) {
	for (uint32_t i = 0; i < str.len; i++) {
		if (str.data[i] == delim) {
			*pos = i;
			return true;
		}
	}
	*pos = str.len;
	return false;
}

void LS_print(LS ls) {
	if (!ls.data) return;
	for (uint32_t i = 0; i < ls.len; i++) {
		putc(ls.data[i], stdout);
	}
}

// i could use str views instead of memory, just one string and views
Production parse_production_str(char* str_in) {
	if (!str_in) { EXIT(); }

	Production prod = {};
	prod.str = str_in;
	LS str = S(str_in);

	char delim = '!';
	uint32_t p = 0;
	// find replacement delim
	if (!find_delim(str, delim, &p)) { EXIT(); }

	if (p >= str.len) { EXIT(); }

	prod.replacement = (LS){str.data + p + 1,
												  str.len - (p + 1)};


	delim = ':';
	str.len = p;
	if (!find_delim(str, delim, &p)) { 
		prod.symbol = (LS){str.data, str.len};
		return prod;
		// done
	}
	prod.symbol = (LS){str.data, p};

	if (p >= str.len) { EXIT(); }

	str.len -= p + 1;
	str.data += p + 1;
	if (!find_delim(str, delim, &p)) { 
		prod.condition = (LS){str.data, str.len};
		return prod;
		// done
	}
	prod.condition = (LS){str.data, p};

	if (p >= str.len) { EXIT(); }

	str.len -= p + 1;
	str.data += p + 1;
	prod.context = (LS){str.data, str.len};

	return prod;
	// done
}

// replacement has been formated to:
// [S{x,y,z} or not ? do i need defaults, i think i do, scaling vals
// first try without args
bool eval_production(Generator* gen, LS replacement) {
	// memcpy replacement into cache
	// i dont even need to copy! i can just use the replace string view
	memcpy(gen->replace.data, replacement.data,
			replacement.len);
	gen->replace.len = replacement.len;
	return true;

	// complex later
	int new_prod_index = 0;	
	int prod_index = 0;	
	bool result = true;
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
bool maybe_replace(Generator* gen, char* symbol) {
	for (int i = 0; i < SSET_LEN(gen->productions); i++) {
		Production* prod = SSet_at(&gen->productions, i);
		if (*prod->symbol.data != *symbol) { continue; }

		// check condition

		// check context

		// replace
		if (eval_production(gen, prod->replacement)) {
			return true;
		} else { EXIT(); }
	}

	return false;

	// test if rule applies
	
	// 	return true;
	// }
}

// returns true if it could finish
bool expand(Generator* gen) {
	// if in_str == NULL eval_production S
	
	
	uint32_t old_index = gen->current_index_old;
	uint32_t new_index = gen->current_index_new;

	if (gen->current_iteration == 0) {
		char s = 'S';
		if (maybe_replace(gen, &s)) {
			memcpy(gen->new_string.data + new_index, gen->replace.data,
					gen->replace.len);
			new_index += gen->replace.len;
			gen->new_string.len += gen->replace.len;
		} else { EXIT(); }
		return true;
		//maybe_replace(S)
	}

	while (old_index < gen->old_string.len) {
		// check time, abort and save current index
		
		// for each symbol maybe_replace
		// if true, add replacement to new_str
		// else copy symbol+block to new_str

		// if (maybe_replace(gen)) {
		// 	memcpy(gen->new_string + new_index, gen->replacement_cache,
		// 				gen->replace.len);
		// 	new_index += gen->replace.len;
		// }
	}
	return false;
}

bool generate_timed(Generator* gen) {
	int iterations = gen->iterations;
	int current_iteration = gen->current_iteration;
	while (current_iteration < iterations) {
		if (expand(gen)) {
			current_iteration++;
		} else {
			return false;
		}
	}
	return true;
}

void reset_generator(Generator* gen) {
		gen->current_iteration = 0;
		gen->current_index_old = 0;
		gen->current_index_new = 0;
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
		}
	}
}
