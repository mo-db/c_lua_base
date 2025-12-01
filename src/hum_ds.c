#include "hum_ds.h"

bool _ArrList_alloc(ArrListInternal* arr_list, uint32_t cap, size_t item_size) {
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

// --- *** sparse set *** ---
bool _SSet_alloc(SSetInternal* sset, uint32_t cap, size_t item_size) {
	sset->dense = malloc(cap * item_size);
	if (!sset->dense) { return false; }

	sset->dense_to_sparse_map = malloc(cap * sizeof(uint32_t));
	if (!sset->dense_to_sparse_map) { return false; }

	sset->sparse = malloc(cap * sizeof(uint32_t));
	if (!sset->sparse) { return false; }

	sset->sparse_free_stack = malloc(cap * sizeof(uint32_t));
	if (!sset->sparse_free_stack) { return false; }

	sset->len = 0;
	sset->cap = cap;
	return true;
}

void _SSet_clear(SSetInternal* sset) {
	sset->len = 0;
	sset->free_stack_len = 0;
}

uint32_t _SSet_push_back(SSetInternal* sset, void* item, size_t item_size) {
	if (sset->len >= sset->cap) {
		return UINT32_MAX;
	}

	// calculate indices for sparse and dense of new item
	uint32_t dense_position = sset->len;
	uint32_t dense_index = dense_position * item_size;
	uint32_t sparse_index = 0;
	if (sset->free_stack_len > 0) {
		sparse_index = sset->sparse_free_stack[--sset->free_stack_len];
	} else {
		sparse_index = dense_position;
	}

	// push_back sparse_index|item to dense
	memcpy(sset->dense + dense_index, item, item_size);

	// set sparse at sparse_index to dense_index
	sset->sparse[sparse_index] = dense_position;
	sset->dense_to_sparse_map[dense_position] = sparse_index;

	sset->len++;

	// sparse_index == unique & stable id
	return sparse_index;
}

void* _SSet_get(SSetInternal* sset, uint32_t sparse_index, size_t item_size) {
	if (sparse_index >= sset->len + sset->free_stack_len) {
		return NULL;
	}
	uint32_t dense_position = sset->sparse[sparse_index];
	return sset->dense + dense_position * item_size;
}


void* _SSet_at(SSetInternal* sset, uint32_t dense_position, size_t item_size) {
	if (dense_position >= sset->len) {
		return NULL;
	}
	return sset->dense + dense_position * item_size;
}

// SSet_get_id
uint32_t _SSet_get_sparse_index(SSetInternal* sset, uint32_t dense_position) {
	if (dense_position >= sset->len) {
		return UINT32_MAX;
	}
	return sset->dense_to_sparse_map[dense_position];
}

bool _SSet_remove(SSetInternal* sset, uint32_t sparse_index_remove_item, size_t item_size) {
	if (sparse_index_remove_item >= sset->len + sset->free_stack_len) {
		return false;
	}

	uint32_t dense_position_remove_item = sset->sparse[sparse_index_remove_item];
	uint32_t dense_index_remove_item = dense_position_remove_item * item_size;

	uint32_t dense_position_last = sset->len - 1;
	uint32_t dense_index_last = dense_position_last * item_size;

	// check if item is last in dense
	if (dense_index_remove_item != dense_index_last) {
		// replace the touple to remove with the last touple in dense
		memcpy(sset->dense + dense_index_remove_item,
					 sset->dense + dense_index_last,
					 item_size);

		// copy the sparse index of dense
		sset->dense_to_sparse_map[dense_position_remove_item] =
			sset->dense_to_sparse_map[dense_position_last];

		// copier den sparse vom to remove item in den sparse vom letzten item
		sset->sparse[sset->dense_to_sparse_map[dense_position_last]] = 
			dense_position_remove_item;

		// push the free sparse_index onto the stack
		sset->sparse_free_stack[sset->free_stack_len++] = sparse_index_remove_item;
	}
	sset->len--;
	return true;
}
