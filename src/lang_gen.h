#include "core.h"
#include "hum_ds.h"
#include "graphics.h"
#include "render.h"

typedef enum : size_t {
  Move = 0,
  Rotate = 1,
  Width = 2,
  Color = 3,
	Stack = 4,
} SymbolCategory ;


// --- generation ---

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

	// defaults
	double move_default;
	double rotate_default;

} Generator;

char* gen_iterate();

bool map_Var_to_SSet(SSet_double* sset, uint32_t* map, Var var);

Production parse_production_str(Generator* gen, char* str_in);

Generator new_generator();



bool get_block(LSView str, char delim, LSView* block);
LS cut_args(const LSView view);

// --- interpretation ---

#define SEG_MAX_NODES 3
typedef struct {
	Vec2 pos;
	double angle;
	double width;
	// use queue later
	uint32_t node_ids_queue[SEG_MAX_NODES];
	uint32_t queue_head;
} InterpreterState;


typedef struct {
	uint32_t node_ids[SEG_MAX_NODES];
	uint32_t node_count;
} Segment;

SSET_DEFINE(SSet_Vec2, Vec2);
SSET_DEFINE(SSet_Segment, Segment);

typedef struct {
	LSView view;
	uint32_t current_index;

	SSet_Vec2 nodes;
	SSet_Segment construct;
	uint32_t segment_node_count;

	InterpreterState state;
	InterpreterState start_state;
	InterpreterState stack[4096];
	uint32_t stack_index;

  bool reset_needed;
	bool redraw_needed;
	bool done_redraw;
  bool done_building;
} Interpreter;


Interpreter new_interpreter(LSView view, InterpreterState state);
bool update_inter(Interpreter* inter);

// inter just temp herer
bool update_generator(Generator* gen);
