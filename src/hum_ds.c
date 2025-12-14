#include "hum_ds.h"

/* --- Dynamic String datatype --- */
void Str_putc(Str *str, const char ch) {
	if (!str) { EXIT(); }
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
	if (!str) { EXIT(); }
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
	if (!str) { EXIT(); }
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
	if (!str) { EXIT(); }
	if (!str->data) return;
	for (uint32_t i = 0; i < str->len; i++) {
		putc(str->data[i], stdout);
	}
	printf("\n");
}

void StrView_print(StrView *view) {
	if (!view) { EXIT(); }
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
	if (!view) { EXIT(); }
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

// --- DA2 ---
bool _DA2_realloc(DA2Internal *da, size_t item_size) {
  if (da->cap == 0) {
    da->cap = 256;
  } else {
    if (da->cap >= UINT32_MAX / (item_size * 2)) {
      return false;
    }
    da->cap *= 2;
  }
  da->data = realloc(da->data, da->cap * item_size);
  if (!da->data) {
    EXIT();
  }
  return true;
}

uint32_t _DA2_push(DA2Internal *da, void *item, size_t item_size) {
	if (!da) { EXIT(); }
	if (da->len >= da->cap) {
		if (!_DA2_realloc(da, item_size)) { return UINT32_MAX; }
	}
	memcpy(da->data + da->len++ * item_size, item, item_size);
	return da->len - 1;
}

void *_DA2_at(DA2Internal *da, uint32_t index, size_t item_size) {
	if (!da) { EXIT(); }
	if (index >= da->len) {
		return NULL;
	}
	return da->data + (index * item_size);
}
