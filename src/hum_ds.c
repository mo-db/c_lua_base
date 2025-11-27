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
