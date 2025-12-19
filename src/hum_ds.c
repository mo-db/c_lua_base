#include "hum_ds.h"

/* --- Dynamic String datatype --- */
bool Str_putc(Str *str, const char ch) {
  if (str->len + 1 >= str->cap) {
    if (!increase_capacity(&str->cap)) { return false; }
    str->data = realloc(str->data, str->cap);
    if (!str->data) { EXIT(); }
    str->data[str->cap - 1] = '\0';
  }
  str->data[str->len++] = ch;
  return true;
}

bool Str_put_cstr(Str *str, const char *cstr) {
	uint32_t cstr_len = strlen_save(cstr);
	for (int i = 0; i < cstr_len; i++) {
		if (!Str_putc(str, cstr[i])) { return false; }
	}
	return true;
}

bool Str_put_view(Str *str, StrView view) {
	for (int i = 0; i < view.len; i++) {
		if (!Str_putc(str, view.data[i])) { return false; }
	}
	return true;
}

bool Str_put_str(Str *dest, const Str *src) {
	for (int i = 0; i < src->len; i++) {
		if (!Str_putc(dest, src->data[i])) { return false; }
	}
	return true;
}

StrView Str_get_view(const Str *str) {
	return (StrView){str->data, str->len};
}

StrView Str_get_view_cstr(char *str) {
	return (StrView){str, strlen_save(str)};
}

void Str_clear(Str* str) { 
	str->len = 0; 
}

Str *Str_new() {
	Str *str = calloc(1, sizeof(Str));
	if (!str) { return NULL; }
	return str;
}

bool Str_free(Str **str) {
	if (!str || !*str) { return false; }
	free((*str)->data);
	free(*str);
	*str = NULL;
	return true;
}

void Str_print(Str *str) {
	if (!str->data) return;
	for (uint32_t i = 0; i < str->len; i++) {
		putc(str->data[i], stdout);
	}
}
void Str_printn(Str *str) {
	if (!str->data) return;
	for (uint32_t i = 0; i < str->len; i++) {
		putc(str->data[i], stdout);
	}
	putc('\n', stdout);
}

void StrView_print(StrView *view) {
	if (!view->data) return;
	for (uint32_t i = 0; i < view->len; i++) {
		putc(view->data[i], stdout);
	}
}

void StrView_printn(StrView *view) {
	if (!view->data) return;
	for (uint32_t i = 0; i < view->len; i++) {
		putc(view->data[i], stdout);
	}
	putc('\n', stdout);
}

bool StrView_offset(StrView* view, uint32_t offset) {
	if (offset > view->len) { return false; }
	view->data += offset;
	view->len -= offset;
	return true;
}

void StrView_trim(StrView* view) {
	if (view->len == 0) { return; }
	while (view->len > 0 && view->data[view->len - 1] == ' ') { view->len--; }
	size_t offset = 0;
	while (offset < view->len && view->data[offset] == ' ') { ++offset; }
	StrView_offset(view, offset);
}

/* --- Sparse Set (of data) --- */
void *SSet_new() {
	void *sset = calloc(1, sizeof(SSetInternal));
	if (!sset) { EXIT(); }
	return sset;
}

bool _SSet_free(void **sset) {
	if (!sset || !*sset) { return false; }
	SSetInternal *sset_internal = (SSetInternal *)(*sset);
	free(sset_internal->data);
	free(sset_internal->pos_to_id_map);
	free(sset_internal->id_to_pos_map);
	free(sset_internal->free_ids);
	free(*(sset));
	*(sset) = NULL;
	return true;
}

void _SSet_clear(SSetInternal *sset) {
	sset->len = 0;
	sset->free_ids_count = 0;
  memset(sset->id_to_pos_map, 0xFF, sset->cap * sizeof(uint32_t));
}

bool _SSet_realloc(SSetInternal *sset, size_t item_size) {
	uint32_t old_cap = sset->cap;
	if (!increase_capacity(&sset->cap)) { return false; }

  sset->data = realloc(sset->data, (uint64_t)sset->cap * item_size);
  if (!sset->data) { EXIT(); }

  sset->pos_to_id_map =
      realloc(sset->pos_to_id_map, (uint64_t)sset->cap * sizeof(uint32_t));
  if (!sset->pos_to_id_map) { EXIT(); }

  sset->id_to_pos_map =
      realloc(sset->id_to_pos_map, (uint64_t)sset->cap * sizeof(uint32_t));
  if (!sset->id_to_pos_map) { EXIT(); }
  memset(sset->id_to_pos_map + old_cap, 0xFF,
				 (sset->cap - old_cap) * sizeof(uint32_t));

  sset->free_ids = realloc(sset->free_ids, (uint64_t)sset->cap * sizeof(uint32_t));
  if (!sset->free_ids) { EXIT(); }
  return true;
}

uint32_t _SSet_push_back(SSetInternal *sset, void *item, size_t item_size) {
	if (sset->len >= sset->cap) {
		if (!_SSet_realloc(sset, item_size)) { return UINT32_MAX; }
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

bool _SSet_emplace_id(SSetInternal *sset, uint32_t id, void *item, size_t item_size) {
	if (sset->len >= sset->cap) {
		if (!_SSet_realloc(sset, item_size)) { return false; }
	}
	// grow cap if necessary
	while (id >= sset->cap) { 
		if (!_SSet_realloc(sset, item_size)) { return false; }
	}
	// if index in free stack, remove it
	for (int i = sset->free_ids_count - 1; i >= 0; i--) {
		if (id == sset->free_ids[i]) { 
			memcpy(sset->free_ids + i,
						 sset->free_ids + i + 1,
						 (sset->free_ids_count - i - 1) * sizeof(uint32_t));
			sset->free_ids_count--;
		}
	}
	// emplace item
	uint32_t data_index = sset->len * item_size;
	memcpy(sset->data + data_index, item, item_size);
	// update the maps
	sset->id_to_pos_map[id] = sset->len;
	sset->pos_to_id_map[sset->len] = id;
	sset->len++;
	return true;
}

bool _SSet_remove(SSetInternal* sset, uint32_t id_to_remove, size_t item_size) {
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

void *_SSet_get(SSetInternal *sset, uint32_t id, size_t item_size) {
	if (id >= sset->cap || sset->id_to_pos_map[id] == UINT32_MAX) {
		return NULL;
	}
	uint32_t pos = sset->id_to_pos_map[id];

	if (pos == UINT32_MAX) {
		return NULL;
	}
	return sset->data + pos * item_size;
}

void *_SSet_at(SSetInternal *sset, uint32_t pos, size_t item_size) {
	if (pos >= sset->len) { return NULL; }
	return sset->data + pos * item_size;
}

/* --- Sparse Set (no managed data) --- */
void *SPSet_new() {
	void *sset = calloc(1, sizeof(SPSetInternal));
	if (!sset) { EXIT(); }
	return sset;
}

bool _SPSet_free(void **sset) {
	if (!sset || !*sset) { return false; }
	SPSetInternal *sset_internal = (SPSetInternal *)(*sset);
	free(sset_internal->data);
	free(sset_internal->pos_to_id_map);
	free(sset_internal->id_to_pos_map);
	free(sset_internal->free_ids);
	free(*(sset));
	*(sset) = NULL;
	return true;
}

void _SPSet_clear(SPSetInternal *sset) {
	sset->len = 0;
	sset->free_ids_count = 0;
  memset(sset->id_to_pos_map, 0xFF, sset->cap * sizeof(uint32_t));
}

bool _SPSet_realloc(SPSetInternal *sset) {
	uint32_t old_cap = sset->cap;
	if (!increase_capacity(&sset->cap)) { return false; }

  sset->data = realloc(sset->data, (uint64_t)sset->cap * (sizeof(void *)));
  if (!sset->data) { EXIT(); }

  sset->pos_to_id_map =
      realloc(sset->pos_to_id_map, (uint64_t)sset->cap * sizeof(uint32_t));
  if (!sset->pos_to_id_map) { EXIT(); }

  sset->id_to_pos_map =
      realloc(sset->id_to_pos_map, (uint64_t)sset->cap * sizeof(uint32_t));
  if (!sset->id_to_pos_map) { EXIT(); }
  memset(sset->id_to_pos_map + old_cap, 0xFF,
				 (sset->cap - old_cap) * sizeof(uint32_t));

  sset->free_ids = realloc(sset->free_ids, (uint64_t)sset->cap * sizeof(uint32_t));
  if (!sset->free_ids) { EXIT(); }
  return true;
}

uint32_t _SPSet_push_back(SPSetInternal *sset, void *item) {
	if (sset->len >= sset->cap) {
		if (!_SPSet_realloc(sset)) { return UINT32_MAX; }
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

bool _SPSet_emplace_id(SPSetInternal *sset, uint32_t id, void *item) {
	if (sset->len >= sset->cap) {
		if (!_SPSet_realloc(sset)) { return false; }
	}
	// grow cap if necessary
	while (id >= sset->cap) { 
		if (!_SPSet_realloc(sset)) { return false; }
	}
	// if index in free stack, remove it
	for (int i = sset->free_ids_count - 1; i >= 0; i--) {
		if (id == sset->free_ids[i]) { 
			memcpy(sset->free_ids + i,
						 sset->free_ids + i + 1,
						 (sset->free_ids_count - i - 1) * sizeof(uint32_t));
			sset->free_ids_count--;
		}
	}
	// emplace item
	sset->data[sset->len] = item;
	// update the maps
	sset->id_to_pos_map[id] = sset->len;
	sset->pos_to_id_map[sset->len] = id;
	sset->len++;
	return true;
}

bool _SPSet_remove(SPSetInternal* sset, uint32_t id_to_remove) {
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

void *_SPSet_get(SPSetInternal *sset, uint32_t id) {
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

void *_SPSet_at(SPSetInternal *sset, uint32_t index) {
	if (index >= sset->len) { return NULL; }
	return sset->data[index];
}

// --- DynArr ---
uint32_t _DynArr_push_back(DynArrInternal *da, void *item, size_t item_size) {
	if (da->len >= da->cap) {
    if (!increase_capacity(&da->cap)) { return false; }
  	da->data = realloc(da->data, (uint64_t)da->cap * item_size);
		if (!da->data) { EXIT(); }
	}
	memcpy(da->data + da->len++ * item_size, item, item_size);
	return da->len - 1;
}

void *_DynArr_at(DynArrInternal *da, uint32_t index, size_t item_size) {
	if (index >= da->len) {
		return NULL;
	}
	return da->data + (index * item_size);
}

void *DynArr_new() {
	void *da = calloc(1, sizeof(DynArrInternal));
	if (!da) { EXIT(); }
	return da;
}

bool _DynArr_free(void **da) {
	if (!da || !*da) { return false; }
	DynArrInternal *da_internal = (DynArrInternal  *)(*da);
	free(da_internal->data);
	free(*(da));
	*(da) = NULL;
	return true;
}
