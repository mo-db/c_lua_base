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
