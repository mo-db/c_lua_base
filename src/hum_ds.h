#pragma once
#include <core.h>

// LS
// - custom string buff type
// - stable address, fixed size, also LSView!
typedef struct {
	char* data;
	uint32_t len;
	uint32_t cap;
} LS;

// does not own the memory, just a fiew like in C++
typedef struct {
	char* data;
	uint32_t len;
} LSView;

uint32_t static inline strlen_save(const char* str) {
	uint32_t len = strnlen(str, UINT32_MAX);
	if (len == 0 || len == UINT32_MAX) { EXIT(); }
	return len;
}
LSView static inline LS_get_view(LS ls) {
	return (LSView){ls.data, ls.len};
}

bool static inline LSView_offset(LSView* view, uint32_t offset) {
	if (offset == 0 || offset > view->len) { return false; }
	view->data += offset;
	view->len -= offset;
	return true;
}

LSView static inline get_view(const char* str) {
	return (LSView){((char*)str), strlen_save(str)};
}

LS LS_new(uint32_t cap);
LS LS_new_from_cstring(const char* str);
void LS_free(LS ls);
void LS_clear(LS* ls);

// returns amount of appended characters, or 0 on failure
uint32_t LS_append(LS* str_dest, LSView str);
bool LS_append_char(LS* str_dest, char ch);
void LS_print(LSView ls);

static inline void LSView_trim(LSView* view) {
	if (view->len == 0) { return; }
	while (view->len > 0 && view->data[view->len - 1] == ' ') {
		view->len--;
	}

	size_t offset = 0;
	while (offset < view->len && view->data[offset] == ' ') {
		++offset;
	}
	LSView_offset(view, offset);
}


// sa
#define SMALL_SEGMENTS_TO_SKIP 6

#define log2i(X) ((uint32_t) (8*sizeof(unsigned long long) \
    - __builtin_clzll((X)) - 1))

typedef struct {
    uint32_t count;
    int used_segments;
    uint8_t* segments[26];
} SegmentArrayInternal;

#define SegmentArray(type) \
    union { \
        SegmentArrayInternal internal; \
        type* payload; \
    }

#define SA_DEFINE(name, type)         \
	typedef union name {                \
		SegmentArrayInternal internal;          \
		type *payload;                  \
	} name;


uint32_t capacity_for_segment_count(int segment_count);
void* _sa_get(SegmentArrayInternal* sa, uint32_t index, size_t item_size);
#define sa_get(sa, index) \
    ((typeof((sa)->payload))_sa_get(&(sa)->internal, \
                                    index,  \
                                    sizeof(*(sa)->payload)))
    

void* _sa_alloc(SegmentArrayInternal* sa, size_t item_size);
#define sa_alloc(sa) \
    (typeof((sa)->payload))_sa_alloc(&(sa)->internal, \
                                     sizeof(*(sa)->payload))


// --- ArrList ---
// + ordered, remove, insert, 
// - unstable pointers, find by identifier slow for big size



// --- ArrayList ---
// ? an array list object must have an id as first element
typedef struct {
	ptrdiff_t len;
	ptrdiff_t cap;
	uint8_t* data;
} ArrListInternal;

#define ArrList(type) \
	union { \
		ArrListInternal internal; \
		type* payload; \
	}

#define AL_DEFINE(name, type)         \
	typedef union name {                \
		ArrayListInternal internal;          \
		type *payload;                  \
	} name;

// A array list needs to be initialized by caller, then alloc data for it
bool _ArrList_alloc(ArrListInternal* arr_list, uint32_t cap, size_t item_size);
#define ArrList_alloc(arr_list, cap) \
	(_ArrList_alloc(&(arr_list)->internal, \
									cap, \
								 	sizeof(*(arr_list)->payload)))

#define ARR_LIST_LEN(alist_2) (alist_2).internal.len
#define ARR_LIST_CAP(alist_2) (alist_2).internal.cap

// the ternary is used for type checking
bool _ArrList_push_back(ArrListInternal* arr_list, void* item, size_t item_size);
#define ArrList_push_back(arr_list, item) \
	(_ArrList_push_back(&(arr_list)->internal, \
										(1 ? (item) : (arr_list)->payload), \
										sizeof(*(arr_list)->payload)))

bool ArrList_pop_back(ArrListInternal* arr_list);

bool _ArrList_remove(ArrListInternal* arr_list, int id, size_t item_size);
#define ArrList_remove(arr_list, index) \
	(_ArrList_remove(&(arr_list)->internal, \
										index,  \
										sizeof(*(arr_list)->payload)))

void* _ArrList_at(ArrListInternal* arr_list, int index, size_t item_size);
#define ArrList_at(arr_list, index) \
	((typeof((arr_list)->payload))_ArrList_at(&(arr_list)->internal, \
																				index,  \
																				sizeof(*(arr_list)->payload)))


// --- *** sparse set *** ---

/*
	- use for items that are continuous memory and dont need to be freed
	- push back does a shallow copie and maps an item to an id
	- items can be removed
*/

typedef struct {
	// amount of active items in dense
	uint32_t len;
	// max items that dense can hold
	uint32_t cap;
	// unordered contigous data-block of item
	uint8_t* dense;
	// corresponding sparse_index for each item in dense
	uint32_t* dense_to_sparse_map;
	// maps uinque id(=sparse_index) to dense_position
	// len of sparse: len + free_stack_len
	uint32_t* sparse;
	// sparse_indices of removed items
	uint32_t* sparse_free_stack;
	// amount of indices in sparse_free_stack
	uint32_t free_stack_len;
} SSetInternal;

#define SSet(type) \
	union { \
		SSetInternal internal; \
		type* payload; \
	}

#define SSET_DEFINE(name, type)         \
	typedef union name {                \
		SSetInternal internal;          \
		type *payload;                  \
	} name;



#define SSET_LEN(sset) (sset).internal.len
#define SSET_CAP(sset) (sset).internal.cap

bool _SSet_alloc(SSetInternal* sset, uint32_t cap, size_t item_size);
#define SSet_alloc(sset, cap) \
	(_SSet_alloc(&(sset)->internal, \
									cap, \
								 	sizeof(*(sset)->payload))) // sizeof, typeof don't evaluate

void _SSet_clear(SSetInternal* sset);
#define SSet_clear(sset) \
	(_SSet_clear(&(sset)->internal))

void _SSet_free(SSetInternal* sset);
#define SSet_free(sset) \
	(_SSet_free(&(sset)->internal))

#define SSET_MAKE_API(name, type)                                           \
static name* name##_new(uint32_t cap) {                                     \
    name* s = (name*)malloc(sizeof(name));                                  \
    if (!s) return NULL;                                                     \
    memset(s, 0, sizeof(*s));                                                \
    if (!_SSet_alloc(&s->internal, cap, sizeof(type))) { free(s); return NULL; } \
    return s;                                                                \
}                                                                           \
static void name##_free(name* s) {                                           \
    if (!s) return;                                                          \
    _SSet_free(&s->internal);                                                \
    free(s);                                                                 \
}

#define NEW_SSET(type) \
	
	


// TODO: make item not a pointer
uint32_t _SSet_push_back(SSetInternal* sset, void* item, size_t item_size); 
#define SSet_push_back(sset, item) \
	(_SSet_push_back(&(sset)->internal, \
									 (1 ? (item) : (sset)->payload), \
									 sizeof(*(sset)->payload)))

// return unstable pointer to item corresponding to unique id
void* _SSet_get(SSetInternal* sset, uint32_t sparse_index, size_t item_size);
#define SSet_get(sset, id) \
	((typeof((sset)->payload))_SSet_get(&(sset)->internal, \
																				id, \
																				sizeof(*(sset)->payload)))

// return unstable pointer to item at dense_position
void* _SSet_at(SSetInternal* sset, uint32_t dense_position, size_t item_size);
#define SSet_at(sset, dense_position) \
	((typeof((sset)->payload))_SSet_at(&(sset)->internal, \
																				dense_position, \
																				sizeof(*(sset)->payload)))
// return id of item at dense_position
uint32_t _SSet_get_sparse_index(SSetInternal* sset, uint32_t dense_position);
#define SSet_id_at(sset, dense_position) \
	(_SSet_get_sparse_index(&(sset)->internal, \
													(dense_position)))

bool _SSet_remove(SSetInternal* sset, uint32_t sparse_index, size_t item_size);
#define SSet_remove(sset, id) \
	(_SSet_remove(&(sset)->internal, \
							id, \
							sizeof(*(sset)->payload)))


// --- sparse set new try ---

/*
	- this is like a pointer database, maps pointers to id's
	- push back copies the pointer to an item
	- a free_function must be provided for the item to remove and free all
*/


typedef struct {
	// amount of active items in dense
	uint32_t len;
	// max items that dense can hold
	uint32_t cap;
	// unordered contigous data-block of item
	void **dense;
	// corresponding sparse_index for each item in dense
	uint32_t *dense_to_sparse_map;
	// maps uinque id(=sparse_index) to dense_position
	// len of sparse: len + free_stack_len
	uint32_t *sparse;
	// sparse_indices of removed items
	uint32_t *sparse_free_stack;
	// amount of indices in sparse_free_stack
	uint32_t free_stack_len;
	uint32_t alloc_size;
} SSInternal;

#define REALLOC_BITS_TO_SKIP 6
#define SS(type) \
	union { \
		SSInternal internal; \
		type* payload; \
	}

#define SS_DEFINE(name, type)         \
	typedef union name {                \
		SSInternal internal;          \
		type *payload;                  \
	} name;

uint32_t _SS_push_back(SSInternal* ss, void* item); 
#define SS_push_back(ss, item) \
	(_SS_push_back(&(ss)->internal, \
									 (1 ? (item) : (ss)->payload)))

bool _SS_remove(SSInternal* ss, uint32_t sparse_index);
#define SS_remove(ss, id) \
	(_SS_remove(&(ss)->internal, \
							id))
// return unstable pointer to item corresponding to unique id
void* _SS_get(SSInternal* sset, uint32_t sparse_index);
#define SS_get(sset, id) \
	((typeof((sset)->payload))_SS_get(&(sset)->internal, \
																				id))

// return unstable pointer to item at dense_position
void* _SS_at(SSInternal* sset, uint32_t dense_position);
#define SS_at(sset, dense_position) \
	((typeof((sset)->payload))_SS_at(&(sset)->internal, \
																				dense_position))
// return id of item at dense_position
uint32_t _SS_get_sparse_index(SSInternal* sset, uint32_t dense_position);
#define SS_id_at(sset, dense_position) \
	(_SS_get_sparse_index(&(sset)->internal, \
													(dense_position)))
