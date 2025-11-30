#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
// #include <math.h>


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

// A array list needs to be initialized by caller, then alloc data for it
bool _ArrList_alloc(ArrListInternal* arr_list, int cap, size_t item_size);
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

union Ident {
	uint8_t* object;
	uint32_t id;
};

// --- Slotmap, unordered, stable pointers/ids, can remove/insert
typedef struct {
	ptrdiff_t len;
	ptrdiff_t cap;
	uint8_t* data;					// cap objects
	uint8_t* dead_map;			// should instead be bitmap, marks indexes as dead
	uint32_t* dead_stack;		// cap ids
	uint32_t* alive_stack;	// cap ids
} Slotmap;


// --- Dynamic Segmented Slotmap ---
typedef struct {
	uint8_t* segments[26];
	int used_segments;
	uint32_t* dead_stack_segments[26];
	int used_dead_stack_segments;
} DSS;


// --- Sparse Set ---
typedef struct {
	char c;
} Blub;


// sizeof(uint32_t) == 4
// sparse_index <=> item_id
// dense_inxes <=> dense_position * item_size * 4
// dense_index == sparse[sparse_index]
typedef struct {
	// amount of active items in dense
	int count;
	// max items that dense can hold
	int cap;
	// unordered contigous data-block touples {sparse_index, item}
	uint8_t* dense;
	uint32_t* sparse_indices_of_dense;
	// array of dense_indices, len of sparse: len + dead_count
	uint32_t* sparse;
	// removed id's
	uint32_t* dead_sparse_indices_stack;
	// count of removed id's
	int dead_count;
} SSetInternal;

#define SSet(type) \
	union { \
		SSetInternal internal; \
		type* payload; \
	}

#define S_SET_COUNT(s_set) (s_set).internal.count
#define S_SET_CAP(s_set) (s_set).internal.cap

bool _SSet_alloc(SSetInternal* s_set, int cap, size_t item_size);
#define SSet_alloc(s_set, cap) \
	(_SSet_alloc(&(s_set)->internal, \
									cap, \
								 	sizeof(*(s_set)->payload))) // why can i do? payload->null

uint32_t _SSet_push_back(SSetInternal* s_set, void* item, size_t item_size); 
#define SSet_push_back(s_set, item) \
	(_SSet_push_back(&(s_set)->internal, \
									 (1 ? (item) : (s_set)->payload), \
									 sizeof(*item))) // could i do like last func?


// return unstable pointer to item corresponding to unique id
void* _SSet_get(SSetInternal* s_set, int sparse_index);
#define SSet_get(s_set, id) \
	((typeof((s_set)->payload))_SSet_get(&(s_set)->internal, \
																				id))

// return unstable pointer to item at dense_position
void* _SSet_at(SSetInternal* s_set, int dense_position, int item_size);
#define SSet_at(s_set, dense_position) \
	((typeof((s_set)->payload))_SSet_at(&(s_set)->internal, \
																				dense_position, \
																				sizeof(*(s_set)->payload)))
// return id of item at dense_position
uint32_t _SSet_get_sparse_index(uint8_t* dense, uint32_t dense_index);
#define SSet_id_at(s_set, dense_position) \
	(_SSet_get_sparse_index((s_set)->internal.dense, \
													(dense_position) * (sizeof(*(s_set)->payload) + 4)))

bool _SSet_remove(SSetInternal* s_set, int sparse_index, int item_size);
#define SSet_remove(s_set, id) \
	(_SSet_remove(&(s_set)->internal, \
							id, \
							sizeof(*(s_set)->payload)))
