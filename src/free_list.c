#include "free_list.h"

const size_t FREE_LIST_SIZE = REG_PHY_FILE_SIZE - REG_LOG_FILE_SIZE;

int init_free_list(free_list_t *free_list) {
  if (free_list == NULL) {
    return ERR_NULL_PTR;
  }

	free_list->list = (queue_t*) calloc(1, sizeof(queue_t));
	if (free_list->list == NULL) {
		return ERR_MEM_ALLOC;
	}

  int err = init_queue(free_list->list, FREE_LIST_SIZE, sizeof(uint8_t));
  if (err != ERR_NONE) {
    free(free_list->list);
    free_list->list = NULL;
    return err;
  }

  for (uint8_t i = REG_LOG_FILE_SIZE; i < REG_LOG_FILE_SIZE + FREE_LIST_SIZE; i++) {
    queue_push(free_list->list, (void*) &i);
  }

  return ERR_NONE;
}

int free_free_list(free_list_t **free_list) {
  if (free_list == NULL || *free_list == NULL) {
    return ERR_NULL_PTR;
  }

  int err = free_queue(&(*free_list)->list);
  if (err != ERR_NONE) {
    return err;
  }

  free(*free_list);
  *free_list = NULL;

  return ERR_NONE;
}

int free_list_push(free_list_t *free_list, uint8_t *reg_idx) {
	if (free_list == NULL || reg_idx == NULL) {
		return ERR_NULL_PTR;
	}

	if (*reg_idx >= REG_PHY_FILE_SIZE) {
		return ERR_INVALID_PHY_REG;
	}

	int err = queue_push(free_list->list, (void*) reg_idx);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int free_list_pop(free_list_t *free_list, uint8_t *reg_idx) {
	if (free_list == NULL || reg_idx == NULL) {
		return ERR_NULL_PTR;
	}

	if (*reg_idx >= REG_PHY_FILE_SIZE) {
		return ERR_INVALID_PHY_REG;
	}

	int err = queue_pop(free_list->list, (void*) reg_idx);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int free_list_search(free_list_t *free_list, uint8_t reg_idx, size_t *index) {
	if (free_list == NULL || index == NULL) {
		return ERR_NULL_PTR;
	}

	if (reg_idx >= REG_PHY_FILE_SIZE) {
		return ERR_INVALID_PHY_REG;
	}

	bool is_empty;
	int err = queue_is_empty(free_list->list, &is_empty);
	if (err != ERR_NONE) {
		return err;
	}

	if (is_empty) {
		return ERR_QUEUE_EMPTY;
	}

	for (size_t i = 0; i < free_list->list->count; i++) {
		uint8_t idx;
		err = queue_peek_logical(free_list->list, i, (void*) &idx);
		if (err != ERR_NONE) {
			return err;
		}

		if (idx == reg_idx) {
			*index = i;
			return ERR_NONE;
		}
	}

	return ERR_QUEUE_EMPTY;
}

int free_list_remove_entry(free_list_t *free_list, uint8_t *reg_idx) {
	if (free_list == NULL || reg_idx == NULL) {
		return ERR_NULL_PTR;
	}

	if (*reg_idx >= REG_PHY_FILE_SIZE) {
		return ERR_INVALID_PHY_REG;
	}

	size_t index;
	int err = free_list_search(free_list, *reg_idx, &index);
	if (err != ERR_NONE) {
		return err;
	}

	err = queue_pop_logical(free_list->list, index, (void*) reg_idx);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int free_list_get_json(
	free_list_t *free_list, yyjson_mut_doc *doc, yyjson_mut_val *root
) {
  if (free_list == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  int err = queue_get_json(
    free_list->list,
		doc, root,
    "FreeList",
    reg_idx_to_json
  );
  if (err != ERR_NONE) {
    fprintf(stderr, "[FL]: Failed to get JSON from queue\n");
    return err;
  }

  return ERR_NONE;
}

void print_free_list(free_list_t *free_list) {
	if (free_list == NULL) {
		printf("[FL]: Free list is NULL\n");
		return;
	}

	printf("[FL]: Free list (from head):\n");
	for (size_t i = 0; i < free_list->list->count; i++) {
		uint8_t reg_idx;
		int err = queue_peek_logical(free_list->list, i, (void*) &reg_idx);
		if (err != ERR_NONE) {
			printf("[FL]: Error peeking free list: %s\n", getErrorDescription(err));
			return;
		}

		printf("[%2lu]: p%2u", i, reg_idx);

		if (i % 4 == 3) {
			printf("\n");
		} else {
			printf(" | ");
		}
	}
		printf("\n");
		printf("[FL]: Free list size: %zu\n", free_list->list->count);
		printf("[FL]: Free list capacity: %zu\n", free_list->list->capacity);
		// printf("[FL]: Free list head: %u\n", free_list->list->head);
		// printf("[FL]: Free list tail: %u\n", free_list->list->tail);
		// printf("[FL]: Free list count: %zu\n", free_list->list->count);
		// printf("[FL]: Free list is empty: %s\n", free_list->list->is_empty ? "true" : "false");
		// printf("[FL]: Free list is full: %s\n", free_list->list->is_full ? "true" : "false");
	printf("[FL]: ********************************\n");
}
