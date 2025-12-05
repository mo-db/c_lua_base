#include "core.h"
#include "hum_ds.h"


typedef struct {
	char ch;
	double value;
} Var;

typedef struct {
	LS str;
	LSView symbol;
	LSView condition;
	LSView context;
	LSView replacement;
} Production;

SSET_DEFINE(SSet_double, double);
SSET_DEFINE(SSet_production, Production);

typedef struct {
	SSet_double vars;
	uint32_t char_id_map[256];
	SSet_production productions;

	LS replacement_buffer;
	LS str0;
	LS str1;
	LSView expanded_string;

	uint32_t current_index;

  bool reset_needed;
  bool done_generating;
  int current_iteration;
  int iterations;
} Generator;

char* gen_iterate();

bool map_Var_to_SSet(SSet_double* sset, uint32_t* map, Var var);


Production parse_production_str(char* str_in);

Generator new_generator();
void update_generator(Generator* gen);


