#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "yyjson.h"
#include "error.h"
#include "data_structures.h"
#include "common.h"
#include "register_table.h"

extern const size_t FREE_LIST_SIZE;

typedef struct {
  queue_t *list;
} free_list_t;

int init_free_list(free_list_t *free_list);
int free_free_list(free_list_t **free_list);

int free_list_push(free_list_t *free_list, uint8_t *reg_idx);
int free_list_pop(free_list_t *free_list, uint8_t *reg_idx);

int free_list_search(free_list_t *free_list, uint8_t reg_idx, size_t *index);
int free_list_remove_entry(free_list_t *free_list, uint8_t *reg_idx);

int free_list_get_json(
	free_list_t *free_list, yyjson_mut_doc *doc, yyjson_mut_val *root
);

void print_free_list(free_list_t *free_list);

#endif // FREE_LIST_H
