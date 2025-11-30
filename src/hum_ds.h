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

// --- Sparse Set ---
// sizeof(uint32_t) == 4
// sparse_index <=> item_id
// dense_inxes <=> dense_position * item_size * 4
// dense_index == sparse[sparse_index]
typedef struct {
	// amount of active items in dense
	int len;
	// max items that dense can hold
	int cap;
	// unordered contigous data-block of item
	uint8_t* dense;
	// corresponding sparse_index for each item in dense
	uint32_t* dense_to_sparse_map;
	// array of dense_indices, len of sparse: len + free_stack_len
	uint32_t* sparse;
	// removed id's
	uint32_t* sparse_free_stack;
	// len of removed id's
	int free_stack_len;
} SSetInternal;

#define SSet(type) \
	union { \
		SSetInternal internal; \
		type* payload; \
	}

#define S_SET_COUNT(s_set) (s_set).internal.len
#define S_SET_CAP(s_set) (s_set).internal.cap

bool _SSet_alloc(SSetInternal* s_set, int cap, size_t item_size);
#define SSet_alloc(s_set, cap) \
	(_SSet_alloc(&(s_set)->internal, \
									cap, \
								 	sizeof(*(s_set)->payload))) // sizeof, typeof don't evaluate

bool _SSet_clear(SSetInternal* s_set);
#define SSet_clear(s_set) \
	(_SSet_clear(&(s_set)->internal))

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
uint32_t _SSet_get_sparse_index(SSetInternal* s_set, uint32_t dense_position);
#define SSet_id_at(s_set, dense_position) \
	(_SSet_get_sparse_index(&(s_set)->internal, \
													(dense_position)))

bool _SSet_remove(SSetInternal* s_set, int sparse_index, int item_size);
#define SSet_remove(s_set, id) \
	(_SSet_remove(&(s_set)->internal, \
							id, \
							sizeof(*(s_set)->payload)))
