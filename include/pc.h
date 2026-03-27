// Program Counter
#ifndef PC_H
#define PC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "yyjson.h"
#include "common.h"
#include "error.h"

typedef struct {
  uint64_t current_pc;
} pc_t;

extern const uint64_t PC_INIT;
extern const uint64_t PC_EXCEPTION;

/**
 * @brief Initialize the program counter
 */
int pc_init(pc_t *pc);

/**
 * @brief Set the program counter to a new value
 */
int pc_set(pc_t *old_pc, pc_t *new_pc);

/**
 * @brief Get the current value of the program counter
 */
int pc_get(pc_t *pc, uint64_t *value);

/**
 * @brief Increment the program counter by the number of fetched
 * instructions per cycle
 */
int pc_inc(pc_t *pc);

/**
 * @brief Set the program counter to an exception state
 */
int pc_set_exception(pc_t *pc);

/**
 * @brief Check if the program counter is in an exception state
 */
int pc_is_exception(pc_t *pc, bool *res);

/**
 * @brief Get the program counter in JSON format
 */
int pc_get_json(pc_t *pc, yyjson_mut_doc *doc, yyjson_mut_val *root);

#endif // PC_H
