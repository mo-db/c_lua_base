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
	WORKING,
	IDLE,
} TimeState;


typedef struct {
	char ch;
	double value;
} Var;

typedef struct {
	Str *str;
	StrView symbol;
	StrView condition;
	StrView context;
	StrView replacement;
} Production;
void production_free(Production* prod);

SSET_DEFINE(SSet_double, double);
SSET_DEFINE(SSet_production, Production);

typedef struct {
	TimeState state;
	SSet_double vars;
	uint32_t char_id_map[256];
	SSet_production productions;

	Str *replacement_buffer;
	Str *str0;
	Str *str1;
	StrView expanded_string;

	uint32_t current_index;

  bool reset_needed;
  bool done_generating;
  int current_iteration;
  int iterations;

	// defaults
	double move_default;
	double rotate_default;

} Generator;

bool generator_add_var(SSet_double* sset, uint32_t* map, double value, char ch);
double *generator_get_var_value(SSet_double* sset, uint32_t* map, char ch);


Production parse_production_str(Generator* gen, const char* str_in);

Generator new_generator();
void reset_generator(Generator* gen);
bool generate_timed(Generator* gen, double frame_time, uint64_t frame_start);


bool get_block(StrView str, char delim, StrView* block);
Str cut_args(const StrView view);

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

typedef struct {
	TimeState state;
	TimeState draw_state;
	uint32_t generator_id;
	StrView view;
	uint32_t current_index;

	SSet_Vec2 nodes;
	SSet_Segment construct;
	uint32_t segment_node_count;

	BuilderState build_state; // TODO, change fetch config lua
	BuilderState start_state;
	BuilderState stack[4096];
	uint32_t stack_index;

  bool reset_needed;
	bool redraw_needed;
	uint32_t current_construct_index; // for drawing

	bool done_redraw;
  bool done_building;
} Builder;


void builder_add_node(Builder *builder , Vec2 *node);
void reset_builder(Builder* builder);
void reset_builder_draw(Builder *builder);

Builder new_builder();

bool build_timed(Builder* builder, double frame_time, uint64_t frame_start);
bool draw_timed(Renderer* renderer, Builder *builder, double frame_time, uint64_t frame_start);

SSET_DEFINE(SSetGenerator, Generator);
SSET_DEFINE(SSetBuilder, Builder);

#define OBJ_MAX 1024
typedef struct {
	SSetGenerator generators;
	SSetBuilder builders;
} LManager;

LManager LManager_new();
void LManager_delete(LManager* manager);

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
