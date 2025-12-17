#pragma once
#include <core.h>

#define DS_LEN(ds) (ds)->internal.len
#define DS_CAP(ds) (ds)->internal.cap
#define INITIAL_CAPACITY 256
static inline bool increase_capacity(uint32_t* cap) {
	if (*cap == UINT32_MAX) { return false; }
	if (*cap == 0) { *cap += INITIAL_CAPACITY ;}
	else { *cap *= 2; }
	return true;
}

/* --- Dynamic String datatype --- */
// lives on the heap
typedef struct {
	char* data;
	uint32_t len;
	uint32_t cap;
} Str;

typedef struct {
	char *data;
	uint32_t len;
} StrView;

Str *Str_new();
bool Str_free(Str **str);
void Str_clear(Str* str);

// -> false if len == UINT32_MAX
bool Str_putc(Str *str, char c);
bool Str_put_cstr(Str *str, const char *cstr);
bool Str_put_view(Str *str, StrView view);

StrView Str_get_view(const Str *str);
StrView Str_get_view_cstr(char *cstr);
void Str_print(Str *str);
void Str_printn(Str *str);

bool StrView_offset(StrView* view, uint32_t offset);
void StrView_trim(StrView* view);
void StrView_print(StrView *view);

/* --- Sparse Set (of data) --- */
// holds shallow copies of objects
// maps those objects to ids while keeping iterability
// -> don't use with objects that own heap memory
typedef struct {
	uint8_t *data;
	uint32_t *pos_to_id_map;
	uint32_t *id_to_pos_map;
	uint32_t *free_ids;
	uint32_t cap;
	uint32_t len;
	uint32_t free_ids_count;
} SSetInternal;

#define SSet(type) \
	union { \
		SSetInternal internal; \
		type* payload; \
	}

#define SSET_DEFINE(name, type) \
	typedef union name { \
		SSetInternal internal; \
		type *payload; \
	} name;

void _SSet_clear(SSetInternal *sset);
#define SSet_clear(sset) (_SSet_clear(&(sset)->internal))

void *SSet_new();

bool _SSet_free(void **sset);
#define SSet_free(sset) (_SSet_free((void **)sset))

uint32_t _SSet_push_back(SSetInternal* sset, void* item, size_t item_size); 
#define SSet_push_back(sset, item) \
	(_SSet_push_back(&(sset)->internal, \
										(1 ? &(item) : (sset)->payload), \
										sizeof(*(sset)->payload)))

bool _SSet_emplace_id(SSetInternal* sset, uint32_t id, void* item, size_t item_size); 
#define SSet_emplace_id(sset, id, item) \
	(_SSet_emplace_id(&(sset)->internal, \
										(id), \
										(1 ? &(item) : (sset)->payload), \
										sizeof(*(sset)->payload)))

bool _SSet_remove(SSetInternal* sset, uint32_t id_to_remove, size_t item_size);
#define SSet_remove(sset, id) \
	(_SSet_remove(&(sset)->internal, \
								 (id), \
								 sizeof(*(sset)->payload)))

// bool SSet_contains(SSetInternal* sset, uint32_t id);
void *_SSet_get(SSetInternal* sset, uint32_t id, size_t item_size);
#define SSet_get(sset, id) \
	((typeof((sset)->payload))_SSet_get(&(sset)->internal, \
								 (id), \
								 sizeof(*(sset)->payload)))

void *_SSet_at(SSetInternal* sset, uint32_t index, size_t item_size);
#define SSet_at(sset, id) \
	((typeof((sset)->payload))_SSet_at(&(sset)->internal, \
								 (id), \
								 sizeof(*(sset)->payload)))

static inline uint32_t _SSet_id_at(SSetInternal *sset, uint32_t index) {
	return sset->pos_to_id_map[index];
}
#define SSet_id_at(sset, index) (_SSet_id_at(&(sset)->internal, (index)))

/* --- Sparse Set (no data) --- */
// only holds pointers to objects
// does not manage their lifetime
// -> user needs to remember to alloc and free objects
typedef struct {
	void **data;
	uint32_t *pos_to_id_map;
	uint32_t *id_to_pos_map;
	uint32_t *free_ids;
	uint32_t cap;
	uint32_t len;
	uint32_t free_ids_count;
} SPSetInternal;

#define SPSet(type) \
	union { \
		SPSetInternal internal; \
		type* payload; \
	}

#define SPSET_DEFINE(name, type) \
	typedef union name { \
		SPSetInternal internal; \
		type *payload; \
	} name;

void _SPSet_clear(SPSetInternal *sset);
#define SPSet_clear(sset) (_SPSet_clear(&(sset)->internal))

void *SPSet_new();

bool _SPSet_free(void **sset);
#define SPSet_free(sset) (_SPSet_free((void **)sset))

uint32_t _SPSet_push_back(SPSetInternal* sset, void* item); 
#define SPSet_push_back(sset, item) \
	(_SPSet_push_back(&(sset)->internal, \
										(1 ? (item) : (sset)->payload)))

// TODO emplace_id()
bool _SPSet_emplace_id(SPSetInternal* sset, uint32_t id, void* item); 
#define SPSet_emplace_id(sset, id, item) \
	(_SPSet_emplace_id(&(sset)->internal, \
										(id), \
										(1 ? (item) : (sset)->payload)))

bool _SPSet_remove(SPSetInternal* sset, uint32_t id_to_remove);
#define SPSet_remove(sset, id) \
	(_SPSet_remove(&(sset)->internal, \
								 (id)))
// bool SPSet_contains(SPSetInternal* sset, uint32_t id);
void *_SPSet_get(SPSetInternal* sset, uint32_t id);
#define SPSet_get(sset, id) \
	((typeof((sset)->payload))_SPSet_get(&(sset)->internal, \
								 (id)))

void *_SPSet_at(SPSetInternal* sset, uint32_t index);
#define SPSet_at(sset, id) \
	((typeof((sset)->payload))_SPSet_at(&(sset)->internal, \
								 (id)))

static inline uint32_t _SPSet_id_at(SPSetInternal *sset, uint32_t index) {
	return sset->pos_to_id_map[index];
}
#define SPSet_id_at(sset, index) \
	(_SPSet_id_at(&(sset)->internal, \
								 (index)))

/* --- dynamic array --- */
// simple dynammic array that can grow
// pointers to items are not stable
typedef struct {
	uint8_t *data;
	uint32_t cap;
	uint32_t len;
} DynArrInternal;

#define DynArr(type) \
	union { \
		DynArrInternal internal; \
		type* payload; \
	}

#define DYNARR_DEFINE(name, type) \
	typedef union name { \
		DynArrInternal internal; \
		type *payload; \
	} name;

void *DynArr_new();

static inline void _DynArr_clear(DynArrInternal *da) { da->len = 0; }
#define DynArr_clear(da) (_DynArr_clear(&(da)->internal))

bool _DynArr_free(void **da);
#define DynArr_free(da) (_DynArr_free((void **)da))

uint32_t _DynArr_push_back(DynArrInternal* da, void* item, size_t item_size); 
#define DynArr_push_back(da, item) \
	(_DynArr_push_back(&(da)->internal, \
										(1 ? &(item) : (da)->payload), \
										sizeof(*(da)->payload)))

void *_DynArr_at(DynArrInternal* da, uint32_t index, size_t item_size); 
#define DynArr_at(da, index) \
	(_DynArr_at(&(da)->internal, \
					 index, \
					 sizeof(*(da)->payload)))
