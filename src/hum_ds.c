#include "hum_ds.h"

/* --- Dynamic String datatype --- */
void Str_putc(Str *str, const char ch) {
	if (str->len + 1 >= str->cap) {
		if (str->cap == 0) { str->cap += 256; }
		else { str->cap *= 2; }
		str->data = realloc(str->data, str->cap);
		if (!str->data) { EXIT(); }
		str->data[str->cap - 1] = '\0';
	}
	str->data[str->len++] = ch;
}

void Str_put_cstr(Str *str, char *cstr) {
	uint32_t cstr_len = strlen_save(cstr);
	for (int i = 0; i < cstr_len; i++) {
		Str_putc(str, cstr[i]);
	}
}

StrView Str_get_view(const Str *str) {
	if (!str) { EXIT(); }
	return (StrView){str, 0, str->len};
}

void Str_clear(Str* str) { 
	str->len = 0; 
}

Str *Str_new() {
	Str *str = malloc(sizeof(Str));
	str->data = NULL;
	str->len = 0;
	str->cap = 0;
	return str;
}

void Str_free(Str *str) {
	if (!str) { EXIT(); }
	if (str->data) { free(str->data); }
	free(str);
	str = NULL;
}

void Str_print(Str *str) {
	if (!str->data) return;
	for (uint32_t i = 0; i < str->len; i++) {
		putc(str->data[i], stdout);
	}
	printf("\n");
}

void StrView_print(StrView *view) {
	if (!view->str) return;
	for (uint32_t i = 0; i < view->len; i++) {
		putc(view->str->data[i], stdout);
	}
	printf("\n");
}

bool StrView_offset(StrView* view, uint32_t offset) {
	if (!view) { EXIT(); }
	if (offset > view->len) { return false; }
	view->offset += offset;
	view->len -= offset;
	return true;
}

void StrView_trim(StrView* view) {
	if (view->len == 0) { return; }
	while (view->len > 0 && view->str->data[view->len - 1] == ' ') {
		view->len--;
	}

	size_t offset = 0;
	while (offset < view->len && view->str->data[offset] == ' ') {
		++offset;
	}
	StrView_offset(view, offset);
}


/* --- Sparse Set (of data) --- */
bool _SSet2_realloc(SSet2Internal *sset, size_t item_size) {
	bool first_alloc = false;
  if (sset->cap == 0) {
    sset->cap = 256;
		first_alloc = true;
  } else {
    if (sset->cap >= UINT32_MAX / (item_size * 2)) {
      return false;
    }
    sset->cap *= 2;
  }
  sset->data = realloc(sset->data, sset->cap * item_size);
  if (!sset->data) {
    EXIT();
  }
  sset->pos_to_id_map =
      realloc(sset->pos_to_id_map, sset->cap * sizeof(uint32_t));
  if (!sset->pos_to_id_map) {
    EXIT();
  }
  sset->id_to_pos_map =
      realloc(sset->id_to_pos_map, sset->cap * sizeof(uint32_t));
  if (!sset->id_to_pos_map) {
    EXIT();
  }
	// set all new id fields to UINT32_MAX
	if (first_alloc) {
  	memset(sset->id_to_pos_map, 0xFF, sset->cap * sizeof(uint32_t));
	} else {
		memset(sset->id_to_pos_map + (sset->cap / 2),
					 0xFF, (sset->cap / 2) * sizeof(uint32_t));
	}
  sset->free_ids = realloc(sset->free_ids, sset->cap * sizeof(uint32_t));
  if (!sset->free_ids) {
    EXIT();
  }
  return true;
}

uint32_t _SSet2_push_back(SSet2Internal *sset, void *item, size_t item_size) {
	if (!sset) { EXIT(); }
	if (sset->len >= sset->cap) {
		if (!_SSet2_realloc(sset, item_size)) { return UINT32_MAX; }
	}

	// calculate indices for sparse and dense of new item
	uint32_t data_index = sset->len * item_size;
	memcpy(sset->data + data_index, item, item_size);
	uint32_t id = sset->len;
	if (sset->free_ids_count > 0) {
		id = sset->free_ids[--sset->free_ids_count];
	}

	// set sparse at sparse_index to dense_index
	sset->id_to_pos_map[id] = sset->len;
	sset->pos_to_id_map[sset->len] = id;
	sset->len++;

	return id;
}

bool _SSet2_emplace_back(SSet2Internal *sset, uint32_t id, void *item, size_t item_size) {
	if (!sset) { EXIT(); }
	if (sset->len >= sset->cap) {
		if (!_SSet2_realloc(sset, item_size)) { return false; }
	}

	if (id >= sset->cap) { return false; }
	bool id_is_free = false;
	if (sset->id_to_pos_map[id] == UINT32_MAX) {
		id_is_free = true;
	} else {
		for (int i = sset->free_ids_count - 1; i >= 0; i--) {
			if (id == sset->free_ids[i]) { id_is_free = true; }
		}
	}
	if (!id_is_free) { return false; }

	uint32_t data_index = sset->len * item_size;
	memcpy(sset->data + data_index, item, item_size);

	sset->id_to_pos_map[id] = sset->len;
	sset->pos_to_id_map[sset->len] = id;
	sset->len++;

	return true;
}

bool _SSet2_remove(SSet2Internal* sset, uint32_t id_to_remove, size_t item_size) {
	if (!sset) { EXIT(); }
	if (id_to_remove >= sset->cap ||
			sset->id_to_pos_map[id_to_remove] == UINT32_MAX) {
		return false;
	}

	uint32_t pos_to_remove = sset->id_to_pos_map[id_to_remove];
	uint32_t index_to_remove = pos_to_remove  * item_size;

	uint32_t pos_last = sset->len - 1;
	uint32_t index_last= pos_last * item_size;

	// check if item is last in dense
	if (pos_to_remove != pos_last) {
		memcpy(sset->data + index_to_remove,
					 sset->data + index_last,
					 item_size);

		// copy the sparse index of dense
		sset->pos_to_id_map[pos_to_remove] =
			sset->pos_to_id_map[pos_last];

		// copier den sparse vom to remove item in den sparse vom letzten item
		sset->id_to_pos_map[sset->pos_to_id_map[pos_last]] = pos_to_remove;

		// push the free sparse_index onto the stack
		sset->free_ids[sset->free_ids_count++] = id_to_remove;
	}
	// mark id as free
	sset->id_to_pos_map[id_to_remove] = UINT32_MAX;
	sset->len--;
	return true;
}

void *_SSet2_get(SSet2Internal *sset, uint32_t id, size_t item_size) {
	if (!sset) { EXIT(); }
	if (id >= sset->cap || sset->id_to_pos_map[id] == UINT32_MAX) {
		return NULL;
	}
	uint32_t pos = sset->id_to_pos_map[id];

	// check if item was freed
	if (pos == UINT32_MAX) {
		return NULL;
	}
	// else return pointer to item
	return sset->data + pos * item_size;
}

void *_SSet2_at(SSet2Internal *sset, uint32_t pos, size_t item_size) {
	if (!sset) { EXIT(); }
	if (pos >= sset->len) {
		return NULL;
	}
	return sset->data + pos * item_size;
}


/* --- Sparse Set (no managed data) --- */
bool _SS2_realloc(SS2Internal *sset) {
	bool first_alloc = false;
  if (sset->cap == 0) {
    sset->cap = 256;
		first_alloc = true;
  } else {
    if (sset->cap >= UINT32_MAX / (sizeof(void *) * 2)) {
      return false;
    }
    sset->cap *= 2;
  }
  sset->data = realloc(sset->data, sset->cap * (sizeof(void *)));
  if (!sset->data) {
    EXIT();
  }
  sset->pos_to_id_map =
      realloc(sset->pos_to_id_map, sset->cap * sizeof(uint32_t));
  if (!sset->pos_to_id_map) {
    EXIT();
  }
  sset->id_to_pos_map =
      realloc(sset->id_to_pos_map, sset->cap * sizeof(uint32_t));
  if (!sset->id_to_pos_map) {
    EXIT();
  }
	// set all new id fields to UINT32_MAX
	if (first_alloc) {
  	memset(sset->id_to_pos_map, 0xFF, sset->cap * sizeof(uint32_t));
	} else {
		memset(sset->id_to_pos_map + (sset->cap / 2),
					 0xFF, (sset->cap / 2) * sizeof(uint32_t));
	}
  sset->free_ids = realloc(sset->free_ids, sset->cap * sizeof(uint32_t));
  if (!sset->free_ids) {
    EXIT();
  }
  return true;
}

uint32_t _SS2_push_back(SS2Internal *sset, void *item) {
	if (!sset) { EXIT(); }
	if (sset->len >= sset->cap) {
		if (!_SS2_realloc(sset)) { return UINT32_MAX; }
	}

	// calculate indices for sparse and dense of new item
	sset->data[sset->len] = item;
	uint32_t id = sset->len;
	if (sset->free_ids_count > 0) {
		id = sset->free_ids[--sset->free_ids_count];
	}

	// set sparse at sparse_index to dense_index
	sset->id_to_pos_map[id] = sset->len;
	sset->pos_to_id_map[sset->len] = id;
	sset->len++;

	return id;
}

bool _SS2_emplace_back(SS2Internal *sset, uint32_t id, void *item) {
	if (!sset) { EXIT(); }
	if (sset->len >= sset->cap) {
		if (!_SS2_realloc(sset)) { return false; }
	}

	if (id >= sset->cap) { return false; }
	bool id_is_free = false;
	if (sset->id_to_pos_map[id] == UINT32_MAX) {
		id_is_free = true;
	} else {
		for (int i = sset->free_ids_count - 1; i >= 0; i--) {
			if (id == sset->free_ids[i]) { id_is_free = true; }
		}
	}
	if (!id_is_free) { return false; }

	sset->data[sset->len] = item;

	sset->id_to_pos_map[id] = sset->len;
	sset->pos_to_id_map[sset->len] = id;
	sset->len++;

	return true;
}

bool _SS2_remove(SS2Internal* sset, uint32_t id_to_remove) {
	if (!sset) { EXIT(); }
	if (id_to_remove >= sset->cap ||
			sset->id_to_pos_map[id_to_remove] == UINT32_MAX) {
		return false;
	}

	uint32_t index_to_remove  = sset->id_to_pos_map[id_to_remove];

	uint32_t index_last = sset->len - 1;

	// check if item is last in dense
	if (index_to_remove != index_last) {
		sset->data[index_to_remove] = sset->data[index_last];

		// copy the sparse index of dense
		sset->pos_to_id_map[index_to_remove] =
			sset->pos_to_id_map[index_last];

		// copier den sparse vom to remove item in den sparse vom letzten item
		sset->id_to_pos_map[sset->pos_to_id_map[index_last]] = index_to_remove;

		// push the free sparse_index onto the stack
		sset->free_ids[sset->free_ids_count++] = id_to_remove;
	}
	// mark id as free
	sset->id_to_pos_map[id_to_remove] = UINT32_MAX;
	sset->len--;
	return true;
}

void *_SS2_get(SS2Internal *sset, uint32_t id) {
	if (!sset) { EXIT(); }
	if (id >= sset->cap || sset->id_to_pos_map[id] == UINT32_MAX) {
		return NULL;
	}
	uint32_t index = sset->id_to_pos_map[id];

	// check if item was freed
	if (index == UINT32_MAX) {
		return NULL;
	}
	// else return pointer to item
	return sset->data[index];
}

void *_SS2_at(SS2Internal *sset, uint32_t index) {
	if (!sset) { EXIT(); }
	if (index >= sset->len) {
		return NULL;
	}
	return sset->data[index];
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
//

void *_SS_new() {
	void *ss = calloc(1, sizeof(SSInternal));
	return ss;
}

bool _SS_realloc(SSInternal* ss) {
	if (ss->cap == 0) {
		ss->cap = 256;
	} else {
		if ( ss->cap >= UINT32_MAX / (sizeof(void *) * 2)) { return false; }
		ss->cap *= 2;
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
	// if (free_item) {
	// 	for (size_t i = 0; i < ss->len; i++) {
	// 		if (!free_item(ss->dense[i])) { return false; }
	// 	}
	// }
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
	}
	// mark sparse as free
	ss->sparse[sparse_index_item_to_remove] = UINT32_MAX;
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

// --- da


void _DA2_append(DAInternal* da, void* item, size_t item_size) {
	if (da->len >= da->cap) {
		if (da->cap == 0) { da->cap = 256; }
		else { da->cap *= 2; }
		da->data = realloc(da->data, da->cap * item_size);
	}
	memcpy(da->data, item, item_size);
	da->len++;
}

// --- SSDouble --



// maybe add size and FIXED/DYNAMIC flag
SSD *SSD_new() {
	SSD *ssd = calloc(1, sizeof(SSD));
	return ssd;
}

bool SSD_free(SSD* ssd) {
	if (!ssd) { return false; }
	if (ssd->data &&
			ssd->index_to_id_map &&
			ssd->id_to_index_map &&
			ssd->free_ids) {
		free(ssd->data);
		free(ssd->index_to_id_map);
		free(ssd->id_to_index_map);
		free(ssd->free_ids);
	}
	free(ssd);
	ssd = NULL;
	return true;
}

bool _SSD_realloc(SSD *ssd) {
  if (ssd->cap == 0) {
    ssd->cap = 256;
  } else {
    if (ssd->cap >= UINT32_MAX / (sizeof(void *) * 2)) {
      return false;
    }
    ssd->cap *= 2;
    ssd->data = realloc(ssd->data, ssd->cap * sizeof(void *));
    if (!ssd->data) {
      EXIT();
    }
    ssd->index_to_id_map =
        realloc(ssd->index_to_id_map, ssd->cap * sizeof(uint32_t));
    if (!ssd->index_to_id_map) {
      EXIT();
    }
    ssd->id_to_index_map =
        realloc(ssd->id_to_index_map, ssd->cap * sizeof(uint32_t));
    if (!ssd->id_to_index_map) {
      EXIT();
    }
    ssd->free_ids = realloc(ssd->free_ids, ssd->cap * sizeof(uint32_t));
    if (!ssd->free_ids) {
      EXIT();
    }
  }
  return true;
}

uint32_t SSD_push_back(SSD *ssd, double item) {
	if (ssd->len >= ssd->cap) {
		if (!_SSD_realloc(ssd)) { return false; }
	}

	// calculate indices for sparse and dense of new item
	ssd->data[ssd->len] = item;
	uint32_t id = ssd->len;
	if (ssd->free_ids_count > 0) {
		id = ssd->free_ids[--ssd->free_ids_count];
	}

	// set sparse at sparse_index to dense_index
	ssd->id_to_index_map[id] = ssd->len;
	ssd->index_to_id_map[ssd->len] = id;
	ssd->len++;

	return id;
}

