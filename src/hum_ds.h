#pragma once
#include <core.h>

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
void Str_free(Str *str);
void Str_clear(Str* str);

bool Str_putc(Str *str, char c);
bool Str_put_cstr(Str *str, const char *cstr);
bool Str_put_view(Str *str, StrView view);

StrView Str_get_view(const Str *str);
StrView Str_get_view_cstr(char *cstr);
void Str_print(Str *str);

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

static inline uint32_t SSet_len(void *sset) {
	if (!sset) { EXIT(); }
	return ((SSetInternal *)sset)->len;
}
static inline uint32_t SSet_cap(void *sset) {
	if (!sset) { EXIT(); }
	return ((SSetInternal *)sset)->cap;
}

static inline void _SSet_clear(SSetInternal *sset) {
	if (!sset) { EXIT(); }
	sset->len = 0;
	sset->free_ids_count = 0;
  memset(sset->id_to_pos_map, 0xFF, sset->cap * sizeof(uint32_t));
}
#define SSet_clear(sset) \
	(_SSet_clear(((sset) ? &(sset)->internal : NULL)))


static inline void *SSet_new() {
	void *ss = calloc(1, sizeof(SSetInternal));
	return ss;
}
#define SSet_free(sset) \
	do { \
		if ((sset) && *(sset)) { \
			free((*(sset))->internal.data); \
			free((*(sset))->internal.pos_to_id_map); \
			free((*(sset))->internal.id_to_pos_map); \
			free((*(sset))->internal.free_ids); \
			free(*(sset)); \
			*(sset) = NULL; \
		} \
	} while(0) 

uint32_t _SSet_push_back(SSetInternal* sset, void* item, size_t item_size); 
#define SSet_push_back(sset, item) \
	(_SSet_push_back(((sset) ? &(sset)->internal : NULL), \
										(1 ? &(item) : (sset)->payload), \
										sizeof(*(sset)->payload)))

bool _SSet_emplace_back(SSetInternal* sset, uint32_t id, void* item, size_t item_size); 
#define SSet_emplace_back(sset, id, item) \
	(_SSet_emplace_back(((sset) ? &(sset)->internal : NULL), \
										(id), \
										(1 ? &(item) : (sset)->payload), \
										sizeof(*(sset)->payload)))

bool _SSet_remove(SSetInternal* sset, uint32_t id_to_remove, size_t item_size);
#define SSet_remove(sset, id) \
	(_SSet_remove(((sset) ? &(sset)->internal : NULL), \
								 (id), \
								 sizeof(*(sset)->payload)))

// bool SSet_contains(SSetInternal* sset, uint32_t id);
void *_SSet_get(SSetInternal* sset, uint32_t id, size_t item_size);
#define SSet_get(sset, id) \
	((typeof((sset)->payload))_SSet_get(((sset) ? &(sset)->internal : NULL), \
								 (id), \
								 sizeof(*(sset)->payload)))

void *_SSet_at(SSetInternal* sset, uint32_t index, size_t item_size);
#define SSet_at(sset, id) \
	((typeof((sset)->payload))_SSet_at(((sset) ? &(sset)->internal : NULL), \
								 (id), \
								 sizeof(*(sset)->payload)))

// TODO: needs type checking
static inline uint32_t SSet_id_at(void *sset, uint32_t index) {
	return ((SSetInternal *)sset)->pos_to_id_map[index];
}

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

static inline uint32_t SPSet_len(void *sset) {
	if (!sset) { EXIT(); }
	return ((SPSetInternal *)sset)->len;
}
static inline uint32_t SPSet_cap(void *sset) {
	if (!sset) { EXIT(); }
	return ((SPSetInternal *)sset)->cap;
}

static inline void _SPSet_clear(SPSetInternal *sset) {
	if (!sset) { EXIT(); }
	sset->len = 0;
	sset->free_ids_count = 0;
  memset(sset->id_to_pos_map, 0xFF, sset->cap * sizeof(uint32_t));
}
#define SPSet_clear(sset) \
	(_SPSet_clear(((sset) ? &(sset)->internal : NULL)))


static inline void *SPSet_new() {
	void *ss = calloc(1, sizeof(SPSetInternal));
	return ss;
}

#define SPSet_free(sset) \
	do { \
		if ((sset) && *(sset)) { \
			free((*(sset))->internal.data); \
			free((*(sset))->internal.pos_to_id_map); \
			free((*(sset))->internal.id_to_pos_map); \
			free((*(sset))->internal.free_ids); \
			free(*(sset)); \
			*(sset) = NULL; \
		} \
	} while(0) 

uint32_t _SPSet_push_back(SPSetInternal* sset, void* item); 
#define SPSet_push_back(sset, item) \
	(_SPSet_push_back(((sset) ? &(sset)->internal : NULL), \
										(1 ? (item) : (sset)->payload)))

bool _SPSet_emplace_back(SPSetInternal* sset, uint32_t id, void* item); 
#define SPSet_emplace_back(sset, id, item) \
	(_SPSet_emplace_back(((sset) ? &(sset)->internal : NULL), \
										(id), \
										(1 ? (item) : (sset)->payload)))

bool _SPSet_remove(SPSetInternal* sset, uint32_t id_to_remove);
#define SPSet_remove(sset, id) \
	(_SPSet_remove(((sset) ? &(sset)->internal : NULL), \
								 (id)))
// bool SPSet_contains(SPSetInternal* sset, uint32_t id);
void *_SPSet_get(SPSetInternal* sset, uint32_t id);
#define SPSet_get(sset, id) \
	((typeof((sset)->payload))_SPSet_get(((sset) ? &(sset)->internal : NULL), \
								 (id)))

void *_SPSet_at(SPSetInternal* sset, uint32_t index);
#define SPSet_at(sset, id) \
	((typeof((sset)->payload))_SPSet_at(((sset) ? &(sset)->internal : NULL), \
								 (id)))
static inline uint32_t SPSet_id_at(void *sset, uint32_t index) {
	return ((SPSetInternal *)sset)->pos_to_id_map[index];
}

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

static inline uint32_t DynArr_len(void *da) {
	if (!da) { EXIT(); }
	return ((DynArrInternal *)da)->len;
}
static inline uint32_t DynArr_cap(void *da) {
	if (!da) { EXIT(); }
	return ((DynArrInternal *)da)->cap;
}

static inline void *DynArr_new() {
	void *da = calloc(1, sizeof(DynArrInternal));
	return da;
}

static inline void _DynArr_clear(DynArrInternal *da) {
	if (!da) { EXIT(); }
	da->len = 0;
}
#define DynArr_clear(da) \
	(_DynArr_clear((da) ? &(da)->internal : NULL))

#define DynArr_free(da) \
	do { \
		if ((da) && *(da)) { \
			free((*(da))->internal.data); \
			free(*(da)); \
			*(da) = NULL; \
		} \
	} while(0) 

uint32_t _DynArr_push(DynArrInternal* da, void* item, size_t item_size); 
#define DynArr_push(da, item) \
	(_DynArr_push(((da) ? &(da)->internal : NULL), \
										(1 ? &(item) : (da)->payload), \
										sizeof(*(da)->payload)))

void *_DynArr_at(DynArrInternal* da, uint32_t index, size_t item_size); 
#define DynArr_at(da, index) \
	(_DynArr_at(((da) ? &(da)->internal : NULL), \
					 index, \
					 sizeof(*(da)->payload)))
