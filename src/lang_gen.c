#include "lang_gen.h"

Generator new_generator() {
	Generator gen = {};
	SSet_alloc(&gen.vars, 128);
	SSet_alloc(&gen.productions, 64);
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
