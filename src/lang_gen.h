#include "core.h"
#include "hum_ds.h"
#include "hum_lua.h"
#include "graphics.h"
#include "render.h"

typedef enum  {
  Move,
  Rotate,
  Width,
  Color,
	Stack,
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
	uint32_t node_ids_queue[SEG_MAX_NODES];
	uint32_t queue_head;
} BuilderState;


typedef struct {
	uint32_t node_ids[SEG_MAX_NODES];
	uint32_t node_count;
} Segment;

SSET_DEFINE(SSet_Vec2, Vec2);
SSET_DEFINE(SSet_Segment, Segment);

typedef enum {
	WORKING,
	IDLE,
} TimeState;

typedef struct {
	LSView view;
	uint32_t current_index;

	SSet_Vec2 nodes;
	SSet_Segment construct;
	uint32_t segment_node_count;

	BuilderState state; // TODO, change fetch config lua
	BuilderState start_state;
	BuilderState stack[4096];
	uint32_t stack_index;

  bool reset_needed;
	bool redraw_needed;
	bool done_redraw;
  bool done_building;
} Builder;


Builder new_interpreter(LSView view, BuilderState state);
bool update_inter(Builder* inter);

// inter just temp herer
bool update_generator(Generator* gen);

SSET_DEFINE(SSetGenerator, Generator);
SSET_DEFINE(SSetBuilder, Builder);

#define OBJ_MAX 1024
typedef struct {
	SSetGenerator generators;
	SSetBuilder builders;
} LManager;

LManager LManager_new();
void LManager_delete(LManager* manager);

void LManager_init_from_config(lua_State* L, LManager* manager);

uint32_t add_generator(LManager* manager);
bool remove_generator(LManager* manager, uint32_t generator_id);
void clear_generators(LManager* manager);

uint32_t add_builder(LManager* manager);
bool remove_builder(LManager* manager, uint32_t builder_id);
void clear_builders(LManager* manager);

int ladd_generator(lua_State* L);
int lremove_generator(lua_State* L);
int ladd_builder(lua_State* L);
int lremove_builder(lua_State* L);
