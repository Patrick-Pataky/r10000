// Issue stage
#ifndef ISSUE_H
#define ISSUE_H

#include <stdint.h>
#include <stdlib.h>
#include "iq.h"
#include "alu.h"
#include "error.h"
#include "common.h"
#include "register_table.h"
#include "active_list.h"
#include "instruction.h"

typedef struct {
	iq_t  *iq;
	alu_t *alus[NUM_ALUS];
	reg_file_t *reg_file;
	reg_map_table_t *reg_map_table;
	busy_bit_table_t *busy_bit_table;
	active_list_t *active_list;
} issue_t;

/**
 * @brief Initialize the issue stage
 */
int init_issue(issue_t *issue, iq_t *iq, alu_t **alus, 
	reg_file_t *reg_file, reg_map_table_t *reg_map_table,
	busy_bit_table_t *busy_bit_table, active_list_t *active_list);

/**
 * @brief Free resources used by the issue stage
 */
int free_issue(issue_t **issue);

/**
 * @brief Perform one cycle of issue operations.
 * 
 * - Takes at most 4 instructions from the integer queue
 * - Issues them to the ALUs
 * 
 * @return The number of instructions issued
 */
int issue_cycle(issue_t *issue);

#endif // ISSUE_H
