#include "rename_dispatch.h"

int init_rename_dispatch(rename_dispatch_t *rename_dispatch, active_list_t *active_list,
	iq_t *iq, decoded_inst_reg_t *dir, fetch_decode_t *fetch_decode,
	reg_file_t *reg_file, reg_map_table_t *reg_map_table, free_list_t *free_list,
	busy_bit_table_t *busy_bit_table, alu_forwarding_path_t fw_paths[NUM_ALUS])
{
	if (rename_dispatch == NULL || active_list == NULL || iq == NULL
		|| dir == NULL || fetch_decode == NULL || reg_file == NULL || reg_map_table == NULL
		|| free_list == NULL || busy_bit_table == NULL) {
		return ERR_NULL_PTR;
	}

	rename_dispatch->active_list = active_list;
	rename_dispatch->iq = iq;
	rename_dispatch->dir = dir;
	rename_dispatch->fetch_decode = fetch_decode;
	rename_dispatch->reg_file = reg_file;
	rename_dispatch->reg_map_table = reg_map_table;
	rename_dispatch->free_list = free_list;
	rename_dispatch->busy_bit_table = busy_bit_table;

	for (int i = 0; i < NUM_ALUS; i++) {
		rename_dispatch->fw_paths[i] = fw_paths[i];
	}

	return ERR_NONE;
}

int free_rename_dispatch(rename_dispatch_t **rename_dispatch) {
	if (rename_dispatch == NULL || *rename_dispatch == NULL) {
		return ERR_NULL_PTR;
	}

	free(*rename_dispatch);
	*rename_dispatch = NULL;

	return ERR_NONE;
}

int rename_operand(rename_dispatch_t *rename_dispatch, operand_t *operand) {
	if (rename_dispatch == NULL || operand == NULL) {
		return ERR_NULL_PTR;
	}

	// Only register operands
	if (operand->type != OPERAND_REG) {
		return ERR_NONE;
	}

	operand->reg.phy_id = rename_dispatch->reg_map_table->logical[operand->reg.log_id];

	return ERR_NONE;
}

int rename_dispatch_update_structures(rename_dispatch_t *rename_dispatch) {
	if (rename_dispatch == NULL) {
		return ERR_NULL_PTR;
	}

	// Update the physical register file
	for (int i = 0; i < NUM_ALUS; i++) {
		instruction_t *instr = rename_dispatch->fw_paths[i];
		if (instr->type == UNKNOWN_TYPE) {
			continue;
		}

		#ifdef DEBUG
			printf("R&D: from forwarding path %d, got instruction: ", i);
			print_instruction(instr);
			printf("\n");
			printf("R&D: %lu = %lu op %lu\n",
				instr->dest.reg.value,
				instr->opA.type == OPERAND_REG ? instr->opA.reg.value : instr->opA.imm,
				instr->opB.type == OPERAND_REG ? instr->opB.reg.value : instr->opB.imm
			);
		#endif

		uint8_t phy_reg_dest_idx = instr->dest.reg.phy_id;
		rename_dispatch->reg_file->physical[phy_reg_dest_idx] = instr->dest.reg.value;
		rename_dispatch->busy_bit_table->is_busy[phy_reg_dest_idx] = instr->is_exception;

		int err = active_list_mark_done(
			rename_dispatch->active_list, instr->pc, true
		);
		if (err != ERR_NONE) {
			return err;
		}

		if (instr->is_exception) {
			err = active_list_mark_exception(
				rename_dispatch->active_list, instr->pc, true
			);
			if (err != ERR_NONE) {
				return err;
			}
		}
	}

	return ERR_NONE;
}

int rename_dispatch_cycle(rename_dispatch_t *rename_dispatch) {
	if (rename_dispatch == NULL) {
		return ERR_NULL_PTR;
	}

	bool apply_backpressure = false;

	// check if enough physical registers are available
	queue_t *free_list = rename_dispatch->free_list->list;
	
	if (free_list->count < INSTRUCTIONS_PER_CYCLE) {
		apply_backpressure = true;

		#ifdef DEBUG
			printf("R&D: backpressure due to free list, as only %zu free registers\n", free_list->count);
		#endif
	}

	// check if enough entries in the active list
	queue_t *active_list = rename_dispatch->active_list->list;
	size_t active_list_size = active_list->capacity - active_list->count;
	if (active_list_size < INSTRUCTIONS_PER_CYCLE) {
		apply_backpressure = true;

		#ifdef DEBUG
			printf("R&D: backpressure due to active list, as only %zu entries\n", active_list_size);
		#endif
	}

	// check if enough entries in the integer queue
	queue_t *iq = rename_dispatch->iq->queue;
	size_t iq_size = iq->capacity - iq->count;
	if (iq_size < INSTRUCTIONS_PER_CYCLE) {
		apply_backpressure = true;

		#ifdef DEBUG
			printf("R&D: backpressure due to IQ, as only %zu entries\n", iq_size);
		#endif
	}

	fetch_decode_t *fetch_decode = rename_dispatch->fetch_decode;
	
	// if backpressure, stop the fetch/decode stage and return
	if (apply_backpressure) {
		fetch_decode->backpressure = true;

		return ERR_NONE;
	}

	// enough resources
	fetch_decode->backpressure = false;

	decoded_inst_reg_t *dir = rename_dispatch->dir;
	if (dir == NULL || dir->queue == NULL) {
		return ERR_NULL_PTR;
	}

	size_t count = dir->queue->count;

	for (size_t i = 0; i < count; i++) {
		instruction_t entry;
		int err = queue_pop(dir->queue, (void*) &entry);
		if (err != ERR_NONE) {
			return err;
		}

		// ***********************
		// Register renaming part*
		// ***********************
		// allocate a new physical register for each operand,
		// or if dependency, reuse register
		err = rename_operand(rename_dispatch, &entry.opA);
		if (err != ERR_NONE) {
			return err;
		}

		err = rename_operand(rename_dispatch, &entry.opB);
		if (err != ERR_NONE) {
			return err;
		}

		// for destination register:
		active_list_entry_t active_entry = {0};
		// - step 1: save old mapping
		active_entry.old_phy_dest = rename_dispatch->reg_map_table->logical[entry.dest.reg.log_id];

		// - step 2: allocate a new physical register
		uint8_t phy_reg_dest_idx;
		err = free_list_pop(rename_dispatch->free_list, &phy_reg_dest_idx);
		if (err != ERR_NONE) {
			return err;
		}

		// - step 3: push to active list
		active_entry.is_done = false;
		active_entry.is_exception = false;
		active_entry.pc.current_pc = entry.pc.current_pc;
		active_entry.log_dest = entry.dest.reg.log_id;
		active_entry.curr_phy_dest = phy_reg_dest_idx;
		err = active_list_push(rename_dispatch->active_list, &active_entry);
		if (err != ERR_NONE) {
			return err;
		}

		// - step 4: update the mapping table
		rename_dispatch->reg_map_table->logical[entry.dest.reg.log_id] = phy_reg_dest_idx;
		entry.dest.reg.phy_id = phy_reg_dest_idx;

		// - step 5: set the busy bit
		rename_dispatch->busy_bit_table->is_busy[phy_reg_dest_idx] = true;

		// *******************
		// Integer queue part*
		// *******************
		bool is_opA_ready = false;
		bool is_opB_ready = false;

		err = is_operand_ready(&entry.opA,
			rename_dispatch->busy_bit_table, rename_dispatch->active_list, &is_opA_ready
		);
		if (err != ERR_NONE) {
			return err;
		}

		err = is_operand_ready(&entry.opB,
			rename_dispatch->busy_bit_table, rename_dispatch->active_list, &is_opB_ready
		);
		if (err != ERR_NONE) {
			return err;
		}

		// if operand is ready, update the value
		if (is_opA_ready && entry.opA.type == OPERAND_REG) {
			entry.opA.reg.value = rename_dispatch->reg_file->physical[entry.opA.reg.phy_id];
		}

		if (is_opB_ready && entry.opB.type == OPERAND_REG) {
			entry.opB.reg.value = rename_dispatch->reg_file->physical[entry.opB.reg.phy_id];
		}

		err = iq_enqueue(
			rename_dispatch->iq, &entry, 
			is_opA_ready, is_opB_ready
		);
		if (err != ERR_NONE) {
			return err;
		}

		#ifdef DEBUG
			printf("R&D: Enqueued instruction");
			print_instruction(&entry);
			printf("\n");
			printf("\t - opA: x%u \t-> p%u\n", entry.opA.reg.log_id, entry.opA.reg.phy_id);
			if (entry.opB.type == OPERAND_REG) {
				printf("\t - opB: x%u \t-> p%u\n", entry.opB.reg.log_id, entry.opB.reg.phy_id);
			} else {
				printf("\t - opB: imm \t-> %ld\n", entry.opB.imm);
			}
			printf("\t - Allocated dest: %u -> p%u\n", entry.dest.reg.log_id, entry.dest.reg.phy_id);
		#endif
	}

	return ERR_NONE;
}
