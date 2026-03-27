// Rename and Dispatch stage
#ifndef RENAME_DISPATCH_H
#define RENAME_DISPATCH_H

#include <stdint.h>
#include <stdlib.h>
#include "active_list.h"
#include "register_table.h"
#include "free_list.h"
#include "error.h"
#include "common.h"
#include "commit.h"
#include "alu.h"
#include "iq.h"
#include "dir.h"
#include "fetch_decode.h"

typedef struct {
	alu_forwarding_path_t fw_paths[NUM_ALUS];
	active_list_t *active_list;
	iq_t *iq;
	decoded_inst_reg_t *dir;
	fetch_decode_t *fetch_decode;
	reg_file_t *reg_file;
	reg_map_table_t *reg_map_table;
	free_list_t *free_list;
	busy_bit_table_t *busy_bit_table;
} rename_dispatch_t;

int init_rename_dispatch(rename_dispatch_t *rename_dispatch, active_list_t *active_list,
		iq_t *iq, decoded_inst_reg_t *dir, fetch_decode_t *fetch_decode,
		reg_file_t *reg_file, reg_map_table_t *reg_map_table, free_list_t *free_list,
		busy_bit_table_t *busy_bit_table, alu_forwarding_path_t fw_paths[NUM_ALUS]);
int free_rename_dispatch(rename_dispatch_t **rename_dispatch);

/**
 * @brief Helper function to handle register renaming for
 * a single operand, and puts the physical register ID in the
 * operand struct.
 */
int rename_operand(rename_dispatch_t *rename_dispatch, operand_t *operand);

/**
 * @brief Observe the results of all functional units through the forwarding
 * paths and update the physical register file as well as the Busy Bit Table
 */
int rename_dispatch_update_structures(rename_dispatch_t *rename_dispatch);

int rename_dispatch_cycle(rename_dispatch_t *rename_dispatch);

#endif // RENAME_DISPATCH_H
