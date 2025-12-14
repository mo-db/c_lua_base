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
	const Str *str;
	uint32_t offset;
	uint32_t len;
} StrView;

Str *Str_new();
void Str_free(Str *str);
void Str_clear(Str* str);

void Str_putc(Str *str, const char ch);
void Str_put_cstr(Str *str, char *cstr);

StrView Str_get_view(const Str *str);
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
} SSet2Internal;

#define SSet2(type) \
	union { \
		SSet2Internal internal; \
		type* payload; \
	}

#define SSET2_DEFINE(name, type) \
	typedef union name { \
		SSet2Internal internal; \
		type *payload; \
	} name;

static inline void *SSet2_new() {
	void *ss = calloc(1, sizeof(SSet2Internal));
	return ss;
}
// bool SSet2_clear(SSet2Internal* sset);
#define SSet2_free(sset) \
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

uint32_t _SSet2_push_back(SSet2Internal* sset, void* item, size_t item_size); 
#define SSet2_push_back(sset, item) \
	(_SSet2_push_back(((sset) ? &(sset)->internal : NULL), \
										(1 ? &(item) : (sset)->payload), \
										sizeof(*(sset)->payload)))

bool _SSet2_emplace_back(SSet2Internal* sset, uint32_t id, void* item, size_t item_size); 
#define SSet2_emplace_back(sset, id, item) \
	(_SSet2_emplace_back(((sset) ? &(sset)->internal : NULL), \
										(id), \
										(1 ? &(item) : (sset)->payload), \
										sizeof(*(sset)->payload)))

bool _SSet2_remove(SSet2Internal* sset, uint32_t id_to_remove, size_t item_size);
#define SSet2_remove(sset, id) \
	(_SSet2_remove(((sset) ? &(sset)->internal : NULL), \
								 (id), \
								 sizeof(*(sset)->payload)))

// bool SSet2_contains(SSet2Internal* sset, uint32_t id);
void *_SSet2_get(SSet2Internal* sset, uint32_t id, size_t item_size);
#define SSet2_get(sset, id) \
	((typeof((sset)->payload))_SSet2_get(((sset) ? &(sset)->internal : NULL), \
								 (id), \
								 sizeof(*(sset)->payload)))

void *_SSet2_at(SSet2Internal* sset, uint32_t index, size_t item_size);
#define SSet2_at(sset, id) \
	((typeof((sset)->payload))_SSet2_at(((sset) ? &(sset)->internal : NULL), \
								 (id), \
								 sizeof(*(sset)->payload)))

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
} SS2Internal;

#define SS2(type) \
	union { \
		SS2Internal internal; \
		type* payload; \
	}

#define SS2_DEFINE(name, type) \
	typedef union name { \
		SS2Internal internal; \
		type *payload; \
	} name;

static inline void *SS2_new() {
	void *ss = calloc(1, sizeof(SS2Internal));
	return ss;
}
// bool SS2_clear(SS2Internal* sset);
#define SS2_free(sset) \
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

uint32_t _SS2_push_back(SS2Internal* sset, void* item); 
#define SS2_push_back(sset, item) \
	(_SS2_push_back(((sset) ? &(sset)->internal : NULL), \
										(1 ? (item) : (sset)->payload)))

bool _SS2_emplace_back(SS2Internal* sset, uint32_t id, void* item); 
#define SS2_emplace_back(sset, id, item) \
	(_SS2_emplace_back(((sset) ? &(sset)->internal : NULL), \
										(id), \
										(1 ? (item) : (sset)->payload)))

bool _SS2_remove(SS2Internal* sset, uint32_t id_to_remove);
#define SS2_remove(sset, id) \
	(_SS2_remove(((sset) ? &(sset)->internal : NULL), \
								 (id)))
// bool SS2_contains(SS2Internal* sset, uint32_t id);
void *_SS2_get(SS2Internal* sset, uint32_t id);
#define SS2_get(sset, id) \
	((typeof((sset)->payload))_SS2_get(((sset) ? &(sset)->internal : NULL), \
								 (id)))

void *_SS2_at(SS2Internal* sset, uint32_t index);
#define SS2_at(sset, id) \
	((typeof((sset)->payload))_SS2_at(((sset) ? &(sset)->internal : NULL), \
								 (id)))


/* --- dynamic array --- */
// simple dynammic array that can grow
// pointers to items are not stable
typedef struct {
	uint8_t *data;
	uint32_t cap;
	uint32_t len;
} DA2Internal;

#define DA2(type) \
	union { \
		DA2Internal internal; \
		type* payload; \
	}

#define DA2_DEFINE(name, type) \
	typedef union name { \
		DA2Internal internal; \
		type *payload; \
	} name;

static inline void *DA2_new() {
	void *da = calloc(1, sizeof(DA2Internal));
	return da;
}
#define DA2_free(da) \
	do { \
		if ((da) && *(da)) { \
			free((*(da))->internal.data); \
			free(*(da)); \
			*(da) = NULL; \
		} \
	} while(0) 

uint32_t _DA2_push(DA2Internal* da, void* item, size_t item_size); 
#define DA2_push(da, item) \
	(_DA2_push(((da) ? &(da)->internal : NULL), \
										(1 ? &(item) : (da)->payload), \
										sizeof(*(da)->payload)))

void *_DA2_at(DA2Internal* da, uint32_t index, size_t item_size); 
#define DA2_at(da, index) \
	(_DA2_at(((da) ? &(da)->internal : NULL), \
					 index, \
					 sizeof(*(da)->payload)))
