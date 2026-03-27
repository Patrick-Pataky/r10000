#include "commit.h"

int init_commit(commit_t *commit, active_list_t *active_list,
	reg_file_t *reg_file, reg_map_table_t *reg_map_table, free_list_t *free_list,
	fetch_decode_t *fetch_decode, exception_status_t *exception_status, pc_t *pc,
	decoded_inst_reg_t *dir, iq_t *iq, alu_t *alus[NUM_ALUS], 
	busy_bit_table_t *busy_bit_table
) {

	if (commit == NULL || active_list == NULL || reg_file == NULL
		  || free_list == NULL || reg_map_table == NULL) {
		return ERR_NULL_PTR;
	}

	commit->active_list = active_list;
	commit->reg_file = reg_file;
	commit->reg_map_table = reg_map_table;
	commit->free_list = free_list;
	commit->exception_mode = false;
	commit->fetch_decode = fetch_decode;
	commit->exception_status = exception_status;
	commit->pc = pc;
	commit->dir = dir;
	commit->iq = iq;
	
	for (size_t i = 0; i < NUM_ALUS; i++) {
		commit->alus[i] = alus[i];
	}

	commit->busy_bit_table = busy_bit_table;
	
	return ERR_NONE;
}

int free_commit(commit_t **commit) {
	if (commit == NULL || *commit == NULL) {
		return ERR_NULL_PTR;
	}

	free(*commit);
	*commit = NULL;

	return ERR_NONE;
}

int commit_cycle(commit_t *commit) {
	if (commit == NULL) {
		return ERR_NULL_PTR;
	}

	// if we are already in exception mode
	if (commit->exception_mode) {
		return commit_exception_mode(commit);
	}

	size_t committed = 0;
	int err;

	/**
	 * Scan the active list for instructions to commit
	 * 
	 * Stop when:
	 * - 4 instructions committed
	 * - instruction not completed
	 * - instruction is an exception
	 */
	while (committed < INSTRUCTIONS_PER_CYCLE) {
		bool is_empty;
		err = queue_is_empty(commit->active_list->list, &is_empty);
		if (err != ERR_NONE) {
			return err;
		}

		// if the active list is empty, we are done
		if (is_empty) {
			break;
		}

		// Go through the active list (ROB), instruction after instruction
		// to commit in order
		active_list_entry_t entry;
		err = active_list_peek_logical(commit->active_list, 0, &entry);
		if (err != ERR_NONE) {
			return err;
		}

		#ifdef DEBUG
			printf("Commit stage: Checking instruction ");
			const instruction_t inst = commit->fetch_decode->program->instructions[entry.pc.current_pc];
			print_instruction(&inst);
			printf("\n");
			print_active_list_entry(&entry);
		#endif

		// stop if instruction is not done
		if (!entry.is_done) {
			break;
		}

		// stop if instruction is an exception
		if (entry.is_exception) {
			// Exception mode for next cycle
			commit->exception_mode = true;
			return commit_set_exception(commit);
		}

		// Can commit:
		// - step 1: free the old physical register
		err = free_list_push(commit->free_list, &entry.old_phy_dest);
		if (err != ERR_NONE) {
			return err;
		}

		// - step 2: pop the entry from the active list
		active_list_entry_t dummy;
		err = active_list_pop_logical(commit->active_list, 0, &dummy);
		if (err != ERR_NONE) {
			return err;
		}

		#ifdef DEBUG
			printf("Commit stage: Committing instruction ");
			print_instruction(&inst);
			printf("\n");
			print_active_list_entry(&entry);
		#endif

		committed++;
	}

	return ERR_NONE;
}

int commit_exception_mode(commit_t *commit) {
	if (commit == NULL) {
		return ERR_NULL_PTR;
	}

	#ifdef DEBUG
		printf("Commit stage: Exception mode\n");
	#endif

	// Step 5: Recover registers (up to 4 instructions per cycle)
	size_t recovered = 0;
	int err;

	active_list_t *active_list = commit->active_list;
	size_t count = active_list->list->count;

	// Clear exception mode if all instructions are recovered
	if (count == 0) {
		commit->exception_status->is_exception = false;
		commit->fetch_decode->backpressure = false;

		#ifdef DEBUG
			printf("Commit stage: Exception mode ended\n");
		#endif
	}
	
	while (recovered < INSTRUCTIONS_PER_CYCLE && count > 0) {
		// pop from active list if possible
		active_list_entry_t entry;

		err = active_list_pop_logical(active_list, count - 1, &entry);
		if (err != ERR_NONE) {
			return err;
		}
		
		// get current physical register and restore the old mapping
		uint8_t current_phy_reg = commit->reg_map_table->logical[entry.log_dest];
		commit->reg_map_table->logical[entry.log_dest] = entry.old_phy_dest;
		
		// push back the current physical register to the free list
		err = free_list_push(commit->free_list, &current_phy_reg);
		if (err != ERR_NONE) {
			return err;
		}
		
		// the old physical register is not busy anymore
		commit->busy_bit_table->is_busy[current_phy_reg] = false;

		recovered++;
		count--;

		#ifdef DEBUG
		printf("Commit exception: Recovered register x%d, old=p%d, current=p%d\n", 
			entry.log_dest, entry.old_phy_dest, current_phy_reg);
		#endif
	}

	return ERR_NONE;
}

int commit_set_exception(commit_t *commit) {
	if (commit == NULL) {
		return ERR_NULL_PTR;
	}

	int err;

	// Step 1: Record PC of the exception, and set the Exception Flag register
	active_list_entry_t entry;
	err = active_list_peek_logical(commit->active_list, 0, &entry);
	if (err != ERR_NONE) {
		return err;
	}

	exception_status_t *exception_status = commit->exception_status;
	exception_status->exception_pc = entry.pc;
	exception_status->is_exception = true;

	// Step 2: Apply backpressure to the fetch stage
	commit->fetch_decode->backpressure = true;

	// Step 3: Set PC to the exception handler
	pc_set_exception(commit->pc);

	// Step 4: Clear the DIR, IQ, and ALUs
	// Clear the DIR
	err = queue_flush(commit->dir->queue);
	if (err != ERR_NONE) {
		return err;
	}

	// Clear the IQ
	err = queue_flush(commit->iq->queue);
	if (err != ERR_NONE) {
		return err;
	}

	// Clear the ALUs
	for (size_t i = 0; i < NUM_ALUS; i++) {
		err = alu_flush(commit->alus[i]);
		if (err != ERR_NONE) {
			return err;
		}
	}

	return ERR_NONE;
}

void print_commit(commit_t *commit) {
	if (commit == NULL) {
		return;
	}

	printf("Commit stage:\n");
}
