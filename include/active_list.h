// Active List
#ifndef ACTIVE_LIST_H
#define ACTIVE_LIST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "yyjson.h"
#include "error.h"
#include "data_structures.h"
#include "register_table.h"
#include "pc.h"

typedef struct {
  bool is_done;
  bool is_exception;
  uint8_t log_dest;
  uint8_t old_phy_dest;
	uint8_t curr_phy_dest;
  pc_t pc;
} active_list_entry_t;

typedef struct {
  queue_t *list;
} active_list_t;

int init_active_list(active_list_t *active_list);
int free_active_list(active_list_t **active_list);

int active_list_push(active_list_t *active_list, active_list_entry_t *entry);

int active_list_peek_logical(active_list_t *active_list, size_t index, active_list_entry_t *entry);
int active_list_pop_logical(active_list_t *active_list, size_t index, active_list_entry_t *entry);
int active_list_update_logical(active_list_t *active_list, size_t index, active_list_entry_t *entry);

int active_list_is_dependent(active_list_t *active_list, uint8_t reg_idx, bool *is_dependent);

/**
 * @brief Iterate through the active list in order and set the
 * first matching entry with a matching logical register to is_done
 */
int active_list_mark_done(active_list_t *active_list, pc_t pc, bool is_done);
int active_list_mark_exception(active_list_t *active_list, pc_t pc, bool is_exception);

int active_list_get_json(active_list_t *active_list, yyjson_mut_doc *doc, yyjson_mut_val *root);

yyjson_mut_val *active_list_entry_to_json(yyjson_mut_doc *doc, void *data);

void print_active_list_entry(active_list_entry_t *entry);

#endif // ACTIVE_LIST_H
