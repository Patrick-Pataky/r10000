// Commit stage
#ifndef COMMIT_H
#define COMMIT_H

#include <stdint.h>
#include <stdlib.h>
#include "active_list.h"
#include "register_table.h"
#include "free_list.h"
#include "fetch_decode.h"
#include "iq.h"
#include "alu.h"
#include "error.h"
#include "common.h"

#define MAX_COMMIT_WIDTH 4

typedef struct {
    active_list_t *active_list;
    reg_file_t *reg_file;
		reg_map_table_t *reg_map_table;
    free_list_t *free_list;
		fetch_decode_t *fetch_decode;
		exception_status_t *exception_status;
		pc_t *pc;
		decoded_inst_reg_t *dir;
		iq_t *iq;
		alu_t *alus[NUM_ALUS];
		busy_bit_table_t *busy_bit_table;
    bool exception_mode;
} commit_t;

int init_commit(commit_t *commit, active_list_t *active_list,
	reg_file_t *reg_file, reg_map_table_t *reg_map_table, free_list_t *free_list,
	fetch_decode_t *fetch_decode, exception_status_t *exception_status, pc_t *pc,
	decoded_inst_reg_t *dir, iq_t *iq, alu_t *alus[NUM_ALUS], 
	busy_bit_table_t *busy_bit_table
);
int free_commit(commit_t **commit);

/**
 * @brief Perform one cycle of commit operations
 */ 
int commit_cycle(commit_t *commit);

/**
 * @brief Called when entering the exception mode.
 *
 * Functionality:
 * 
 * - Recover registers (up to 4 instructions per cycle)
 */
int commit_exception_mode(commit_t *commit);

/**
 * @brief Set the exception mode
 * 
 * Functionality:
 * 
 * - Record PC of the exception, and set the Exception Flag register
 * 
 * - Apply backpressure to the fetch stage
 * 
 * - Set PC to the exception handler
 * 
 * - Clear the DIR, IQ, and ALUs
 */
int commit_set_exception(commit_t *commit);

void print_commit(commit_t *commit);

#endif // COMMIT_H
