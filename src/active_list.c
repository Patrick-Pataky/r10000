#include "active_list.h"

int init_active_list(active_list_t *active_list) {
  if (active_list == NULL) {
    return ERR_NULL_PTR;
  }

  active_list->list = (queue_t*) calloc(1, sizeof(queue_t));
  if (active_list->list == NULL) {
    return ERR_MEM_ALLOC;
  }

  int err = init_queue(active_list->list, REG_LOG_FILE_SIZE, sizeof(active_list_entry_t));
  if (err != ERR_NONE) {
    free(active_list->list);
    active_list->list = NULL;
    return err;
  }

  return ERR_NONE;
}

int free_active_list(active_list_t **active_list) {
  if (active_list == NULL || *active_list == NULL) {
    return ERR_NULL_PTR;
  }

  int err = free_queue(&(*active_list)->list);
  if (err != ERR_NONE) {
    return err;
  }

  free(*active_list);
  *active_list = NULL;

  return ERR_NONE;
}

int active_list_push(active_list_t *active_list, active_list_entry_t *entry) {
	if (active_list == NULL || entry == NULL) {
		return ERR_NULL_PTR;
	}

	int err = queue_push(active_list->list, (void*) entry);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int active_list_peek_logical(active_list_t *active_list, size_t index, active_list_entry_t *entry) {
	if (active_list == NULL || entry == NULL) {
		return ERR_NULL_PTR;
	}

	if (index >= active_list->list->count) {
		return ERR_QUEUE_INVALID_INDEX;
	}

	int err = queue_peek_logical(active_list->list, index, (void*) entry);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int active_list_pop_logical(active_list_t *active_list, size_t index, active_list_entry_t *entry) {
	if (active_list == NULL || entry == NULL) {
		return ERR_NULL_PTR;
	}

	if (index >= active_list->list->count) {
		return ERR_QUEUE_INVALID_INDEX;
	}

	int err = queue_pop_logical(active_list->list, index, (void*) entry);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int active_list_update_logical(active_list_t *active_list, size_t index, active_list_entry_t *entry) {
	if (active_list == NULL || entry == NULL) {
		return ERR_NULL_PTR;
	}

	if (index >= active_list->list->count) {
		return ERR_QUEUE_INVALID_INDEX;
	}

	int err = queue_update_logical(active_list->list, index, (void*) entry);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int active_list_is_dependent(active_list_t *active_list, uint8_t reg_idx, bool *is_dependent) {
	if (active_list == NULL || is_dependent == NULL) {
		return ERR_NULL_PTR;
	}

	*is_dependent = false;

	size_t count = active_list->list->count;

	for (size_t i = 0; i < count; i++) {
		active_list_entry_t entry;
		int err = queue_peek_logical(active_list->list, i, (void*) &entry);
		if (err != ERR_NONE) {
			return err;
		}

		if (entry.log_dest == reg_idx && !entry.is_done) {
			*is_dependent = true;
			break;
		}
	}

	return ERR_NONE;
}

int active_list_mark_done(active_list_t *active_list, pc_t pc, bool is_done) {
	if (active_list == NULL) {
		return ERR_NULL_PTR;
	}

	size_t count = active_list->list->count;

	for (size_t i = 0; i < count; i++) {
		active_list_entry_t entry;
		int err = queue_peek_logical(active_list->list, i, (void*) &entry);
		if (err != ERR_NONE) {
			return err;
		}

		// TODO:dirty hack
		if (entry.pc.current_pc == pc.current_pc) {
			entry.is_done = is_done;
			err = queue_update_logical(active_list->list, i, (void*) &entry);
			if (err != ERR_NONE) {
				return err;
			}
			break;
		}
	}

	return ERR_NONE;
}

int active_list_mark_exception(active_list_t *active_list, pc_t pc, bool is_exception) {
	if (active_list == NULL) {
		return ERR_NULL_PTR;
	}

	size_t count = active_list->list->count;

	for (size_t i = 0; i < count; i++) {
		active_list_entry_t entry;
		int err = queue_peek_logical(active_list->list, i, (void*) &entry);
		if (err != ERR_NONE) {
			return err;
		}

		if (entry.pc.current_pc == pc.current_pc) {
			entry.is_exception = is_exception;
			err = queue_update_logical(active_list->list, i, (void*) &entry);
			if (err != ERR_NONE) {
				return err;
			}
			break;
		}
	}

	return ERR_NONE;
}

int active_list_get_json(active_list_t *active_list, yyjson_mut_doc *doc, yyjson_mut_val *root) {
  if (active_list == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  int err = queue_get_json(
    active_list->list,
    doc, root,
    "ActiveList",
    active_list_entry_to_json
  );
  if (err != ERR_NONE) {
    fprintf(stderr, "[AL]: Failed to get JSON from active list\n");
    return err;
  }

  return ERR_NONE;
}

yyjson_mut_val *active_list_entry_to_json(yyjson_mut_doc *doc, void *data) {
  if (data == NULL || doc == NULL) {
    return NULL;
  }

  active_list_entry_t *entry = (active_list_entry_t *)data;

	yyjson_mut_val *json = yyjson_mut_obj(doc);
	if (json == NULL) {
		fprintf(stderr, "[AL]: Failed to create JSON object\n");
		return NULL;
	}

	yyjson_mut_obj_add_bool(doc, json, "Done", entry->is_done);
	yyjson_mut_obj_add_bool(doc, json, "Exception", entry->is_exception);
	yyjson_mut_obj_add_uint(doc, json, "LogicalDestination", entry->log_dest);
	yyjson_mut_obj_add_uint(doc, json, "OldDestination", entry->old_phy_dest);
	yyjson_mut_obj_add_uint(doc, json, "PC", entry->pc.current_pc);

  return json;
}

void print_active_list_entry(active_list_entry_t *entry) {
	if (entry == NULL) {
		printf("[AL]: NULL entry\n");
		return;
	}

	printf("[AL]: Entry: \n");
	printf("  - PC: %lu\n", entry->pc.current_pc);
	printf("  - Done: %s\n", entry->is_done ? "true" : "false");
	printf("  - Exception: %s\n", entry->is_exception ? "true" : "false");
	printf("  - Logical Destination: %d\n", entry->log_dest);
	printf("  - Old Destination: %d\n", entry->old_phy_dest);
}
