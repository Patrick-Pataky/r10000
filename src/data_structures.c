#include "data_structures.h"

int init_queue(queue_t *queue, size_t capacity, size_t size_of_element) {
  if (queue == NULL) {
    return ERR_NULL_PTR;
  }

  if (capacity == 0 || size_of_element == 0) {
    return ERR_QUEUE_INIT;
  }

  if (capacity > SIZE_MAX / size_of_element) {
    return ERR_BUFFER_OVERFLOW;
  }

  queue->head = 0;
  queue->tail = 0;
  queue->count = 0;
  queue->capacity = capacity;
  queue->size_of_element = size_of_element;
  queue->data = calloc(queue->capacity, size_of_element);

  if (queue->data == NULL) {
    return ERR_MEM_ALLOC;
  }

  return ERR_NONE;
}

int free_queue(queue_t **queue) {
  if (queue == NULL || *queue == NULL) {
    return ERR_NULL_PTR;
  }

  if ((*queue)->data != NULL) {
    free((*queue)->data);
  }

  free(*queue);
  *queue = NULL;

  return ERR_NONE;
}

int queue_push(queue_t *queue, void *data) {
  if (queue == NULL || data == NULL) {
    return ERR_NULL_PTR;
  }

  if (queue->count == queue->capacity) {
    return ERR_QUEUE_FULL;
  }

  memcpy(
    (char *)queue->data + (queue->tail * queue->size_of_element),
    data,
    queue->size_of_element
  );

  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->count++;

  return ERR_NONE;
}

int queue_pop(queue_t *queue, void *data) {
  if (queue == NULL || data == NULL) {
    return ERR_NULL_PTR;
  }

  if (queue->count == 0) {
    return ERR_QUEUE_EMPTY;
  }

  memcpy(
    data,
    (char *)queue->data + (queue->head * queue->size_of_element),
    queue->size_of_element
  );

  queue->head = (queue->head + 1) % queue->capacity;
  queue->count--;

  return ERR_NONE;
}

int queue_peek_logical(const queue_t* queue, size_t index, void *data) {
  if (queue == NULL || data == NULL) {
    return ERR_NULL_PTR;
  }

  if (index >= queue->count) {
    return ERR_QUEUE_INVALID_INDEX;
  }

  size_t logical_index = (queue->head + index) % queue->capacity;

  memcpy(
    data,
    (char *)queue->data + (logical_index * queue->size_of_element),
    queue->size_of_element
  );

  return ERR_NONE;
}

int queue_pop_logical(queue_t *queue, size_t index, void *data) {
	if (queue == NULL || data == NULL) {
		return ERR_NULL_PTR;
	}

	if (index >= queue->count) {
		return ERR_QUEUE_INVALID_INDEX;
	}

	size_t logical_index = (queue->head + index) % queue->capacity;

	memcpy(
		data,
		(char *)queue->data + (logical_index * queue->size_of_element),
		queue->size_of_element
	);

	// Shift elements to the left
	for (size_t i = index; i < queue->count - 1; i++) {
		size_t next_logical_index = (queue->head + i + 1) % queue->capacity;

		// copy current element to the next logical index
		memcpy(
			(char *)queue->data + (logical_index * queue->size_of_element),
			(char *)queue->data + (next_logical_index * queue->size_of_element),
			queue->size_of_element
		);

		logical_index = next_logical_index;
	}

	// decrement tail and count
	queue->tail = (queue->tail - 1 + queue->capacity) % queue->capacity;
	queue->count--;

	return ERR_NONE;
}

int queue_update_logical(queue_t *queue, size_t index, void *data) {
	if (queue == NULL || data == NULL) {
		return ERR_NULL_PTR;
	}

	if (index >= queue->count) {
		return ERR_QUEUE_INVALID_INDEX;
	}

	size_t logical_index = (queue->head + index) % queue->capacity;

	memcpy(
		(char *)queue->data + (logical_index * queue->size_of_element),
		data,
		queue->size_of_element
	);

	return ERR_NONE;
}

int queue_is_empty(queue_t *queue, bool *is_empty) {
	if (queue == NULL || is_empty == NULL) {
		return ERR_NULL_PTR;
	}

	*is_empty = (queue->count == 0);
	return ERR_NONE;
}

int queue_flush(queue_t *queue) {
	if (queue == NULL) {
		return ERR_NULL_PTR;
	}

	queue->head = 0;
	queue->tail = 0;
	queue->count = 0;

	return ERR_NONE;
}

void print_queue(queue_t *queue) {
  if (queue == NULL) {
    return;
  }

  printf("Queue contents:\n");

  for (size_t i = 0; i < queue->count; i++) {
    size_t logical_index = (queue->head + i) % queue->capacity;
    void *data = (char *)queue->data + (logical_index * queue->size_of_element);
    printf("%x, ", *(int *)data);
  }

  printf("\n");
}

int queue_get_json(
  queue_t *queue, yyjson_mut_doc *doc, yyjson_mut_val *root, const char *name,
  yyjson_mut_val *(*element_to_json)(yyjson_mut_doc *doc, void *data))
{
  if (queue == NULL || doc == NULL || root == NULL || name == NULL 
			|| element_to_json == NULL) {
    return ERR_NULL_PTR;
  }

  yyjson_mut_val *queue_array = yyjson_mut_arr(doc);
  if (queue_array == NULL) {
    fprintf(stderr, "[%s]: Failed to create JSON array\n", name);
    return ERR_MEM_ALLOC;
  }

  for (size_t i = 0; i < queue->count; i++) {
    size_t logical_index = (queue->head + i) % queue->capacity;
    void *data = (char *)queue->data + (logical_index * queue->size_of_element);

    yyjson_mut_val *element_json = element_to_json(doc, data);
    if (element_json == NULL) {
      fprintf(stderr, "[%s]: Failed to convert element to JSON\n", name);
      return ERR_INVALID_JSON;
    }

    yyjson_mut_arr_append(queue_array, element_json);
  }

  if (!yyjson_mut_obj_add_val(doc, root, name, queue_array)) {
    fprintf(stderr, "[%s]: Failed to set JSON object\n", name);
    return ERR_INVALID_JSON;
  }

  return ERR_NONE;
}

int init_exception_status(exception_status_t *status) {
  if (status == NULL) {
    return ERR_NULL_PTR;
  }

  status->is_exception = false;
  status->exception_pc.current_pc = 0;

  return ERR_NONE;
}

int free_exception_status(exception_status_t **status) {
  if (status == NULL || *status == NULL) {
    return ERR_NULL_PTR;
  }

  free(*status);
  *status = NULL;

  return ERR_NONE;
}

int exception_status_get_json(
	exception_status_t *status, yyjson_mut_doc *doc, yyjson_mut_val *root
) {
  if (status == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

	if (!yyjson_mut_obj_add_uint(
		doc, root, "ExceptionPC", status->exception_pc.current_pc
	)) {
		fprintf(stderr, "Failed to add ExceptionPC to JSON\n");
		return ERR_INVALID_JSON;
	}

	if (!yyjson_mut_obj_add_bool(
		doc, root, "Exception", status->is_exception
	)) {
		fprintf(stderr, "Failed to add Exception to JSON\n");
		return ERR_INVALID_JSON;
	}

  return ERR_NONE;
}
