#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "yyjson.h"
#include "error.h"
#include "pc.h"

// Fifo Queue
typedef struct {
  size_t head;
  size_t tail;
  size_t count;
  size_t capacity;
  size_t size_of_element;
  void **data;
} queue_t;

int init_queue(queue_t *queue, size_t capacity, size_t size_of_element);
int free_queue(queue_t **queue);

int queue_push(queue_t *queue, void *data);
int queue_pop(queue_t *queue, void *data);

/**
 * @brief Peek at the element at the given index in the queue
 * without removing it, at a logical index (0 = head, 1 = next, etc.)
 */
int queue_peek_logical(const queue_t* queue, size_t index, void *data);

/**
 * @brief Removes the element at the given index in the queue
 * at a logical index (0 = head, 1 = next, etc.)
 */
int queue_pop_logical(queue_t *queue, size_t index, void *data);

/**
 * @brief Update (overwrite) the element at the given index in the queue
 * at a logical index (0 = head, 1 = next, etc.)
 */
int queue_update_logical(queue_t *queue, size_t index, void *data);

int queue_is_empty(queue_t *queue, bool *is_empty);
int queue_flush(queue_t *queue);

void print_queue(queue_t *queue);
int queue_get_json(
  queue_t *queue, yyjson_mut_doc *doc, yyjson_mut_val *root, const char *name,
  yyjson_mut_val *(*element_to_json)(yyjson_mut_doc *doc, void *data)
);
// End of Fifo Queue

// --- Core Data Structures ---

typedef struct {
  bool is_exception;
  pc_t exception_pc;
} exception_status_t;

int init_exception_status(exception_status_t *status);
int free_exception_status(exception_status_t **status);

int exception_status_get_json(
	exception_status_t *status, yyjson_mut_doc *doc, yyjson_mut_val *root
);

#endif // DATA_STRUCTS_H
