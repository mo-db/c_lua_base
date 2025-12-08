#include "hum_ds.h"

// LS
LS LS_new(uint32_t cap) {
	LS ls = {};
	ls.data = malloc(cap);
	ls.len = 0;
	ls.cap = cap;
	if (!ls.data) { EXIT(); }
	return ls;
}

LS LS_new_from_cstring(const char* str) {
	uint32_t len = strlen_save(str);
	LS ls = LS_new(len);
	memcpy(ls.data, str, len);
	ls.len = len;
	return ls;
}

void LS_free(LS ls) {
	if (!ls.data) { EXIT(); }
	free(ls.data);
}

void LS_clear(LS* ls) {
	ls->len = 0;
}

void LS_print(LSView ls) {
	if (!ls.data) return;
	for (uint32_t i = 0; i < ls.len; i++) {
		putc(ls.data[i], stdout);
	}
	printf("\n");
}
uint32_t LS_append(LS* str_dest, LSView str) {
	if (str_dest->len < str_dest->cap && 
			str.len < str_dest->cap - str_dest->len) {
		memcpy(str_dest->data + str_dest->len, str.data, str.len);
		str_dest->len += str.len;
		return str.len;
	} else {
		return 0;
	}
}
bool LS_append_char(LS* str_dest, char ch) {
	if (str_dest->len < str_dest->cap) {
		str_dest->data[str_dest->len] = ch;
		str_dest->len++;
		return true;
	} else {
		return false;
	}
}




// sa


uint32_t capacity_for_segment_count(int segment_count) {
    return ((1 << SMALL_SEGMENTS_TO_SKIP) << segment_count) 
        - (1 << SMALL_SEGMENTS_TO_SKIP);
}

void *_sa_get(SegmentArrayInternal *sa, uint32_t index, size_t item_size) {
    int segment = log2i((index >> SMALL_SEGMENTS_TO_SKIP) + 1);
    uint32_t slot = index - capacity_for_segment_count(segment);
    return sa->segments[segment] + item_size*slot;
}

void *_sa_alloc(SegmentArrayInternal *sa, size_t item_size) {
    if (sa->count >= capacity_for_segment_count(sa->used_segments)) {
        size_t slots_in_segment = (1 << SMALL_SEGMENTS_TO_SKIP) << sa->used_segments;
        size_t segment_size = item_size * slots_in_segment;
        sa->segments[sa->used_segments] = malloc(segment_size);
        sa->used_segments++;
    }

    sa->count++;
    return _sa_get(sa, sa->count-1, item_size);
}


// allist

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
	sset->free_stack_len = 0;
	sset->cap = cap;
	return true;
}

void _SSet_clear(SSetInternal* sset) {
	sset->len = 0;
	sset->free_stack_len = 0;
}

void _SSet_free(SSetInternal* sset) {
	free(sset->dense);
	free(sset->dense_to_sparse_map);
	free(sset->sparse);
	free(sset->sparse_free_stack);
	sset->len = 0;
	sset->free_stack_len = 0;
	sset->cap = 0;
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

	// check if item was freed
	if (dense_position == sset->cap) {
		return NULL;
	}
	// else return pointer to item
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
		// mark sparse as free
		sset->sparse[sparse_index_remove_item] = sset->cap;
	}
	sset->len--;
	return true;
}



// --- *** sset that takes pointers to malloced objects ***
bool _SS_realloc(SSInternal* ss) {
	if (ss->cap == 0) {
		ss->cap = 1 << REALLOC_BITS_TO_SKIP;
	} else {
		if ( ss->cap >= UINT32_MAX / (sizeof(void *) * 2)) { return false; }
		ss->cap <<= 1;
	}

	ss->dense = realloc(ss->dense, ss->cap * sizeof(void *));
	if (!ss->dense) { return false; }

	ss->dense_to_sparse_map = realloc(ss->dense_to_sparse_map, ss->cap * sizeof(uint32_t));
	if (!ss->dense_to_sparse_map) { EXIT(); }

	ss->sparse = realloc(ss->sparse, ss->cap * sizeof(uint32_t));
	if (!ss->sparse) { EXIT(); }

	ss->sparse_free_stack = realloc(ss->sparse_free_stack, ss->cap * sizeof(uint32_t));
	if (!ss->sparse_free_stack) { EXIT(); }
	return true;
}

// this way i can free complex objects, but i cant free a ss inside a ss
bool SS_free(SSInternal* ss, bool free_item(void *)) {
	if (free_item) {
		for (size_t i = 0; i < ss->len; i++) {
			if (!free_item(ss->dense[i])) { return false; }
		}
	}
	free(ss->dense);
	free(ss->dense_to_sparse_map);
	free(ss->sparse);
	free(ss->sparse_free_stack);
	free(ss);
	return true;
}

bool SS_free_inner_SS(void *item) {
    if (!item) return true;
    SSInternal *inner = (SSInternal*)item;
    return SS_free(inner, SS_free_inner_SS);
}

uint32_t _SS_push_back(SSInternal* ss, void* item) {
	if (ss->len == ss->cap) {
		if (!_SS_realloc(ss)) {
			return UINT32_MAX;
		}
	}

	// calculate indices for sparse and dense of new item
	ss->dense[ss->len] = item;
	uint32_t sparse_index = ss->len;
	if (ss->free_stack_len > 0) {
		sparse_index = ss->sparse_free_stack[--ss->free_stack_len];
	}

	// set sparse at sparse_index to dense_index
	ss->sparse[sparse_index] = ss->len;
	ss->dense_to_sparse_map[ss->len] = sparse_index;
	ss->len++;

	return sparse_index;
}

bool _SS_remove(SSInternal* ss, uint32_t sparse_index_item_to_remove) {
	if (sparse_index_item_to_remove >= ss->len + ss->free_stack_len ||
			ss->sparse[sparse_index_item_to_remove] == UINT32_MAX) {
		return false;
	}

	uint32_t dense_index_item_to_remove =
		ss->sparse[sparse_index_item_to_remove];
	uint32_t dense_index_last = ss->len - 1;

	// free?

	// check if item is last in dense
	if (dense_index_item_to_remove != dense_index_last) {
		// replace dense_to_remove with last dense
		ss->dense[dense_index_item_to_remove] = ss->dense[dense_index_last];

		// update dense_to_sparse_map
		ss->dense_to_sparse_map[dense_index_item_to_remove] =
			ss->dense_to_sparse_map[dense_index_last];

		// copy spars_index_item_to_remove into sparse of last item in dense
		ss->sparse[ss->dense_to_sparse_map[dense_index_last]] = 
			dense_index_item_to_remove;

		// push the free sparse_index onto the stack
		ss->sparse_free_stack[ss->free_stack_len++] =
			sparse_index_item_to_remove;
		// mark sparse as free
		ss->sparse[sparse_index_item_to_remove] = UINT32_MAX;
	}
	ss->len--;
	return true;
}


void* _SS_get(SSInternal* sset, uint32_t sparse_index) {
	if (sparse_index >= sset->len + sset->free_stack_len) {
		return NULL;
	}
	uint32_t dense_index = sset->sparse[sparse_index];

	// check if item was freed
	if (dense_index == UINT32_MAX) {
		return NULL;
	}
	// else return pointer to item
	return sset->dense[dense_index];
}

void* _SS_at(SSInternal* sset, uint32_t dense_position) {
	if (dense_position >= sset->len) {
		return NULL;
	}
	return sset->dense[dense_position];
}

// SS_get_id
uint32_t _SS_get_sparse_index(SSInternal* sset, uint32_t dense_position) {
	if (dense_position >= sset->len) {
		return UINT32_MAX;
	}
	return sset->dense_to_sparse_map[dense_position];
}
