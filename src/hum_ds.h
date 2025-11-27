#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
// #include <math.h>

// --- ArrayList ---
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

