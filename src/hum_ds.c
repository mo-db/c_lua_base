#include "hum_ds.h"

bool _ArrList_alloc(ArrListInternal* arr_list, int cap, size_t item_size) {
	arr_list->data = malloc(cap * item_size);
	if (!arr_list->data) { return false; }
	arr_list->len = 0;
	arr_list->cap = cap;

	return true;
}

bool _ArrList_push_back(ArrListInternal *arr_list, void *item,
                        size_t item_size) {
	if (arr_list->len >= arr_list->cap) {
		return false;
	}
  uint8_t *dest = arr_list->data + arr_list->len * item_size;
  memcpy(dest, item, item_size);
  arr_list->len++;
	return true;
}

bool ArrList_pop_back(ArrListInternal *arr_list) {
  if (arr_list->len <= 0) {
    return false;
	}
	arr_list->len--;
	return true;
}

// remove element at specific index
bool _ArrList_remove(ArrListInternal *arr_list, int index, size_t item_size) {
  if (arr_list->len <= 0 || index >= arr_list->len || index < 0) {
    return false;
  }
  arr_list->len--;
  if (index == arr_list->len) {
    return true;
  }
  memcpy(arr_list->data + index * item_size,
         arr_list->data + (index + 1) * item_size,
         (arr_list->len - index) * item_size);
  return true;
}

// void *_ArrList_get(ArrListInternal *arr_list, int id, size_t item_size) {
//   for (int i = 0; i < arr_list->len; i++) {
//     uint8_t *item = arr_list->data + i * item_size;
//     if (id == *(int *)item) {
//       return item;
//     }
//   }
//   return arr_list->data;
// }

void *_ArrList_at(ArrListInternal *arr_list, int index, size_t item_size) {
  return arr_list->data + index * item_size;
}

// return id by serch
int _ArrList_find_id() {}

// --- sparse set ---
bool _SSet_alloc(SSetInternal* s_set, int cap, size_t item_size) {
	s_set->dense = malloc(cap * (item_size + 4));
	if (!s_set->dense) { return false; }
	s_set->sparse = malloc(cap * 4);
	if (!s_set->sparse) { return false; }
	s_set->dead_sparse_indices_stack = malloc(cap * 4);
	if (!s_set->dead_sparse_indices_stack) { return false; }
	s_set->count = 0;
	s_set->cap = cap;
	return true;
}


uint32_t _SSet_push_back(SSetInternal* s_set, void* item, size_t item_size) {
	if (s_set->count >= s_set->cap) {
		return false;
	}

	// calculate indices for sparse and dense of new item
	uint32_t dense_index = s_set->count * (item_size + 4);
	uint32_t sparse_index = 0;
	if (s_set->dead_count > 0) {
		// printf("SSet_push_back: pop dead sparse from stack\n");
		sparse_index = s_set->dead_sparse_indices_stack[--s_set->dead_count];
	} else {
		sparse_index = s_set->count;
	}

	// push_back sparse_index|item to dense
	uint8_t* dest = s_set->dense + dense_index;
	memcpy(dest, &sparse_index, 4);
	memcpy(dest + 4, item, item_size);

	// set sparse at sparse_index to dense_index
	s_set->sparse[sparse_index] = dense_index;

	s_set->count++;

	// sparse_index == unique & stable id
	return sparse_index;
}

void* _SSet_get(SSetInternal* s_set, int sparse_index) {
	if (sparse_index >= s_set->count + s_set->dead_count) {
		return NULL;
	}
	return s_set->dense + s_set->sparse[sparse_index] + 4;
}


void* _SSet_at(SSetInternal* s_set, int dense_position, int item_size) {
	if (dense_position >= s_set->count) {
		return NULL;
	}
	return s_set->dense + dense_position * (item_size + 4) + 4;
}

// SSet_get_id
uint32_t _SSet_get_sparse_index(uint8_t* dense, uint32_t dense_index) {
	return (uint32_t)(*(dense + dense_index));
}

bool _SSet_remove(SSetInternal* s_set, int sparse_index, int item_size) {
	if (sparse_index >= s_set->count) {
		return NULL;
	}
	uint32_t dense_index = s_set->sparse[sparse_index];
	uint32_t last_dense_index = (s_set->count - 1) * (item_size + 4);

	// check if item is last in dense
	if (dense_index != last_dense_index) {
		// replace the touple to remove with the last touple in dense
		memcpy(s_set->dense + dense_index,
					 s_set->dense + last_dense_index,
					 item_size + 4);
		// copier den sparse vom to remove item in den sparse vom letzten item
		memcpy(s_set->sparse + _SSet_get_sparse_index(s_set->dense, last_dense_index),
						s_set->sparse + sparse_index,
						4);
		// push the dead sparse_index onto the stack
		s_set->dead_sparse_indices_stack[s_set->dead_count++] = sparse_index;
	}
	s_set->count--;
	return true;
}
