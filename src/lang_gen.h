#include "core.h"
#include "hum_ds.h"

#define S(str) \
	(LS){(str), strlen(str)}

typedef struct {
	char* data;
	uint32_t len;
} LS;

typedef struct {
	char ch;
	double value;
} Var;

#define STR_LEN 1024

typedef struct {
	char* str;
	LS symbol;
	LS condition;
	LS context;
	LS replacement;
} Production;

SSET_DEFINE(SSet_double, double);
SSET_DEFINE(SSet_production, Production);

typedef struct {
	SSet_double vars;
	uint32_t char_id_map[128];
	SSet_production productions;

	char replacement_cache[4096];
	LS replace;

	char old_string_cache[50000];
	LS old_string;
	char new_string_cache[50000];
	LS new_string;

	uint32_t current_index_new;
	uint32_t current_index_old;

  bool reset_needed;
  bool done_generating;
  int current_iteration;
  int iterations;
} Generator;

char* gen_iterate();

bool map_Var_to_SSet(SSet_double* sset, uint32_t* map, Var var);


Production parse_production_str(char* str_in);
void LS_print(LS ls);

Generator new_generator();
void update_generator(Generator* gen);


