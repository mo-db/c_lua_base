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
typedef enum {
	OFFLINE,
	WORKING,
	IDLE,
} TimeState;

typedef struct {
	char ch;
	double value;
} Var;

typedef struct {
	Str *symbol;
	Str *condition;
	Str *context;
	Str *replacement;
} Production;


SSET_DEFINE(SSet_double, double);
SPSET_DEFINE(SPSet_production, Production);

typedef struct {
	SSet_double *vars;
	SPSet_production *productions;
	Str *replacement_buffer;
	Str *str0;
	Str *str1;

	// just a pointer not a string that has memory
	Str *expanded_string;
	uint32_t current_index;

	TimeState state;

	// events
  bool reset_needed;
  bool done_generating; // not needed
	bool enable;
	bool disable;

  int current_iteration;
  int iterations;

	// defaults
	double move_default;
	double rotate_default;

} Generator;

Production *production_new();
void production_free(Production *production);
bool format_and_check_production(Generator *generator, Production *production);
Production *parse_production_str(Str* production_str);


Generator *generator_new();
void generator_free(Generator *gen);
void reset_generator(Generator* gen);
bool generate_timed(Generator* gen, double frame_time, uint64_t frame_start);


bool get_block(StrView str, char delim, StrView* block);

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

DYNARR_DEFINE(DynArrVec2, Vec2);
DYNARR_DEFINE(DynArrSegment, Segment);

typedef struct {
	TimeState state;
	uint32_t generator_id;
	TimeState draw_state;
	Str *lstring_non_owning;
	uint32_t current_index;

	DynArrVec2 *nodes;
	DynArrSegment *construct;
	uint32_t segment_node_count;

	BuilderState build_state; // TODO, change fetch config lua
	BuilderState start_state;
	BuilderState stack[4096];
	uint32_t stack_index;

	// events
  bool reset_needed;
	bool redraw_needed;

	uint32_t current_construct_index; // for drawing

	bool done_redraw;
  bool done_building;
} Builder;


void builder_add_node(Builder *builder , Vec2 node);
void reset_builder(Builder* builder);
void reset_builder_draw(Builder *builder);

Builder *builder_new();
void builder_free(Builder *builder);

bool build_timed(Builder* builder, double frame_time, uint64_t frame_start);
bool draw_timed(Renderer* renderer, Builder *builder, double frame_time, uint64_t frame_start);

SPSET_DEFINE(SPSetGenerator, Generator);
SPSET_DEFINE(SPSetBuilder, Builder);

typedef struct {
	SPSetGenerator *generators;
	SPSetBuilder *builders;
} LManager;

LManager *LManager_new();
void LManager_free(LManager* manager);

bool update_lsystem(Renderer *renderer, LManager *manager, double frame_time, uint64_t frame_start);
void reconfigure_system(lua_State *L, LManager *manager);

// also clear buffer somehow
void redraw_all(LManager *manager);

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
