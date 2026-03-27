// Integer Queue
#ifndef IQ_H
#define IQ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "yyjson.h"
#include "data_structures.h"
#include "instruction.h"
#include "error.h"
#include "register_table.h"
#include "active_list.h"
#include "alu.h"

extern const size_t IQ_CAPACITY;

typedef struct {
  instruction_t instr;
  bool is_opA_ready;
  bool is_opB_ready;
} iq_entry_t;

typedef struct {
  queue_t *queue;
} iq_t;

int iq_init(iq_t *iq);
int iq_free(iq_t **iq);

int iq_enqueue(iq_t *iq, instruction_t *instr, bool is_opA_ready, bool is_opB_ready);

/**
 * @brief Dequeue an instruction from the integer queue,
 * at logical index (0 = head, 1 = next, etc.)
 */
int iq_pop_logical(iq_t *iq, size_t index, iq_entry_t *entry);

/**
 * @brief Update an entry
 * at logical index (0 = head, 1 = next, etc.)
 */
int iq_update_logical(iq_t *iq, size_t index, iq_entry_t *entry);

int iq_get_json(iq_t *iq, yyjson_mut_doc *doc, yyjson_mut_val *root);

/**
 * @brief Check if the instruction at the given index is ready
 */
int iq_entry_is_ready(iq_entry_t *entry, bool *is_ready);

/**
 * @brief Get the executed instruction from the forwarding path
 * and search for possible dependencies in the integer queue, and
 * set the corresponding opA and opB ready flags.
 */
int iq_set_fw(iq_t *iq, alu_forwarding_path_t *forwarding_path);

/**
 * @brief Iterate through the integer queue and check through the busy bit
 * table if the instruction is ready to be issued.
 */
int iq_refresh_ready(
	iq_t *iq, reg_file_t *reg_file, reg_map_table_t *reg_map_table, 
	busy_bit_table_t *busy_bit_table, active_list_t *active_list
);

/**
 * @brief Get the first n ready instructions from the integer queue
 * (ordered from the oldest to the newest instruction), and remove them.
 * 
 * @param entries Array to store the ready instructions, assumed to be
 * 							  large enough to hold n iq_entries.
 * @param num_ready Pointer to store the number of ready instructions.
 */
int iq_get_first_n_ready(iq_t *iq, size_t n, iq_entry_t *entries, size_t *num_ready);

void print_iq_entry(iq_entry_t *entry);

#endif // IQ_H
