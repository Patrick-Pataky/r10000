#include "processor.h"

const uint32_t INSTRUCTIONS_PER_CYCLE = 4;
bool ready_to_finish = false;

int init_processor(processor_t *processor, program_t *program) {
	int err;
	
	if (processor == NULL || program == NULL) {
		return ERR_NULL_PTR;
	}

	// Initialize the processor state
	processor->state.finished = false;
	processor->state.program = *program;
	
	// Initialize PC
	err = pc_init(&processor->state.pc);
	if (err != ERR_NONE) return err;
	
	// Initialize exception status
	err = init_exception_status(&processor->state.exception_status);
	if (err != ERR_NONE) return err;
	
	// Initialize register file
	err = reset_reg_file(&processor->state.reg_file);
	if (err != ERR_NONE) return err;
	
	// Initialize register map table
	err = reset_reg_map_table(&processor->state.reg_map_table);
	if (err != ERR_NONE) return err;
	
	// Initialize free list
	err = init_free_list(&processor->state.free_list);
	if (err != ERR_NONE) return err;
	
	// Initialize busy bit table
	err = reset_busy_bit_table(&processor->state.busy_bit_table);
	if (err != ERR_NONE) return err;
	
	// Initialize active list
	err = init_active_list(&processor->state.active_list);
	if (err != ERR_NONE) return err;
	
	// Pipeline stages
	// DIR
	processor->dir = (decoded_inst_reg_t *) calloc(1, sizeof(decoded_inst_reg_t));
	if (processor->dir == NULL) return ERR_MEM_ALLOC;
	err = init_decoded_inst_reg(processor->dir);
	if (err != ERR_NONE) return err;

	// Fetch & Decode stage
	processor->fetch_decode = (fetch_decode_t *) calloc(1, sizeof(fetch_decode_t));
	if (processor->fetch_decode == NULL) return ERR_MEM_ALLOC;
	err = init_fetch_decode(processor->fetch_decode, &processor->state.pc, 
												 processor->dir, &processor->state.program);
	if (err != ERR_NONE) return err;
	
	// Issue Queue
	processor->iq = (iq_t *) calloc(1, sizeof(iq_t));
	if (processor->iq == NULL) return ERR_MEM_ALLOC;
	err = iq_init(processor->iq);
	if (err != ERR_NONE) return err;
	
	// ALUs
	processor->fw_paths = (alu_forwarding_path_t *) calloc(NUM_ALUS, sizeof(alu_forwarding_path_t));
	if (processor->fw_paths == NULL) return ERR_MEM_ALLOC;

	for (int i = 0; i < NUM_ALUS; i++) {
		processor->alus[i] = (alu_t *) calloc(1, sizeof(alu_t));
		if (processor->alus[i] == NULL) return ERR_MEM_ALLOC;
		err = alu_init(processor->alus[i]);
		if (err != ERR_NONE) return err;

		processor->fw_paths[i] = processor->alus[i]->forwarding_path;
	}

	// Issue stage
	processor->issue = (issue_t *) calloc(1, sizeof(issue_t));
	if (processor->issue == NULL) return ERR_MEM_ALLOC;
	err = init_issue(processor->issue, processor->iq, processor->alus,
									&processor->state.reg_file, &processor->state.reg_map_table,
									&processor->state.busy_bit_table, &processor->state.active_list);
	if (err != ERR_NONE) return err;
	
	// Rename/Dispatch stage
	processor->rename_dispatch = (rename_dispatch_t *) calloc(1, sizeof(rename_dispatch_t));
	if (processor->rename_dispatch == NULL) return ERR_MEM_ALLOC;
	err = init_rename_dispatch(processor->rename_dispatch, &processor->state.active_list,
													 processor->iq, processor->dir, processor->fetch_decode,
													 &processor->state.reg_file, &processor->state.reg_map_table,
													 &processor->state.free_list, &processor->state.busy_bit_table,
													 processor->fw_paths);
	if (err != ERR_NONE) return err;
	
	// Commit stage
	processor->commit = (commit_t *) calloc(1, sizeof(commit_t));
	if (processor->commit == NULL) return ERR_MEM_ALLOC;
	err = init_commit(processor->commit, &processor->state.active_list,
									 &processor->state.reg_file, &processor->state.reg_map_table,
									 &processor->state.free_list, processor->fetch_decode,
									 &processor->state.exception_status, &processor->state.pc,
									 processor->dir, processor->iq, processor->alus,
									 &processor->state.busy_bit_table);
	if (err != ERR_NONE) return err;

	#ifdef DEBUG
		printf("Processor initialized successfully\n");
		printf("\t - PC: %lu\n", processor->state.pc.current_pc);
		printf("\t - Exception status: %s\n", processor->state.exception_status.is_exception ? "true" : "false");
		printf("**********************************\n");
	#endif
	
	return ERR_NONE;
}

int free_processor(processor_t **processor) {
	if (processor == NULL || *processor == NULL) {
		return ERR_NULL_PTR;
	}

	processor_t *p = *processor;
	int err;

	// Free Commit stage
	if (p->commit != NULL) {
		err = free_commit(&p->commit);
		if (err != ERR_NONE) return err;
	}

	// Free Rename/Dispatch stage
	if (p->rename_dispatch != NULL) {
		err = free_rename_dispatch(&p->rename_dispatch);
		if (err != ERR_NONE) return err;
	}

	// Free Issue stage
	if (p->issue != NULL) {
		err = free_issue(&p->issue);
		if (err != ERR_NONE) return err;
	}

	// Free ALUs
	free(p->fw_paths);
	p->fw_paths = NULL;

	for (int i = 0; i < NUM_ALUS; i++) {
		if (p->alus[i] != NULL) {
			free(p->alus[i]);
			p->alus[i] = NULL;
		}
	}

	// Free IQ
	if (p->iq != NULL) {
		err = iq_free(&p->iq);
		if (err != ERR_NONE) return err;
	}

	// Free Fetch & Decode stage
	if (p->fetch_decode != NULL) {
		err = free_fetch_decode(&p->fetch_decode);
		if (err != ERR_NONE) return err;
	}

	// Free DIR
	if (p->dir != NULL) {
		err = free_decoded_inst_reg(&p->dir);
		if (err != ERR_NONE) return err;
	}

	// Free the processor itself
	free(p);
	*processor = NULL;

	return ERR_NONE;
}

void processor_cycle(processor_t* processor) {
	if (processor == NULL) {
		return;
	}

	int err;

	#ifdef DEBUG
		printf("* Executing new cycle: PC: %lu\n", processor->state.pc.current_pc);
	#endif

	// Start simulating from the end of the pipeline
	// Commit stage
	err = commit_cycle(processor->commit);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error in commit stage: %s\n", getErrorDescription(err));
		return;
	}

	for (size_t i = 0; i < NUM_ALUS; i++) {
		err = alu_cycle(processor->alus[i]);
		if (err != ERR_NONE) {
			fprintf(stderr, "Error in ALU %zu: %s\n", i, getErrorDescription(err));
			return;
		}

		#ifdef DEBUG
			print_alu(processor->alus[i], i);
		#endif
	}

	// Update RF with ALU results
	err = rename_dispatch_update_structures(processor->rename_dispatch);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error in rename/dispatch update: %s\n", getErrorDescription(err));
		return;
	}

	// Issue stage
	err = issue_cycle(processor->issue);
	if (err < ERR_NONE) {
		fprintf(stderr, "Error in issue stage: %s\n", getErrorDescription(err));
		return;
	}

	// Rename/Dispatch stage
	err = rename_dispatch_cycle(processor->rename_dispatch);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error in rename/dispatch stage: %s\n", getErrorDescription(err));
		return;
	}

	// Fetch & Decode stage
	int fetched_instr = fetch_decode_cycle(processor->fetch_decode);
	if (fetched_instr < ERR_NONE) {
		fprintf(stderr, "Error in fetch/decode stage: %s\n", getErrorDescription(err));
		return;
	}

	// Check if we are finished (= no more instructions + active list empty)
	bool is_active_list_empty = false;
	err = queue_is_empty(processor->state.active_list.list, &is_active_list_empty);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error checking active list: %s\n", getErrorDescription(err));
		return;
	}

	if (ready_to_finish) {
		processor->state.finished = true;
	}

	if (fetched_instr == 0 && is_active_list_empty
			&& processor->state.exception_status.is_exception == false) {
		ready_to_finish = true;
	}
}

void run_simulation(processor_t* processor, yyjson_mut_doc *doc, yyjson_mut_val *root) {
	if (processor == NULL || doc == NULL || root == NULL) {
		return;
	}

	size_t cycle = 0;

	do {
		yyjson_mut_val *state_obj = yyjson_mut_obj(doc);

		int err = processor_get_json(processor, doc, state_obj);
		if (err != ERR_NONE) {
			fprintf(stderr, "Error getting JSON: %s\n", getErrorDescription(err));
			return;
		}

		yyjson_mut_arr_append(root, state_obj);

		if (cycle == 6) {
			// break;
		}

		processor_cycle(processor);
		cycle++;
	} while (!processor->state.finished);
}

int processor_get_json(processor_t *processor, yyjson_mut_doc *doc, yyjson_mut_val *root) {
	if (processor == NULL || doc == NULL || root == NULL) {
		return ERR_NULL_PTR;
	}

	int err;

	// PC
	err = pc_get_json(&processor->state.pc, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting PC JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// Exception status
	err = exception_status_get_json(&processor->state.exception_status, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting exception status JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// Register file
	err = reg_file_get_json(&processor->state.reg_file, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting register file JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// Register map table
	err = reg_map_table_get_json(&processor->state.reg_map_table, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting register map table JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// Free list
	err = free_list_get_json(&processor->state.free_list, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting free list JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// Busy bit table
	err = busy_bit_table_get_json(&processor->state.busy_bit_table, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting busy bit table JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// Active list
	err = active_list_get_json(&processor->state.active_list, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting active list JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// DIR (Decoded Instruction Register)
	err = decoded_inst_reg_get_json(processor->dir, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting DIR JSON: %s\n", getErrorDescription(err));
		return err;
	}

	// IQ (Integer Queue)
	err = iq_get_json(processor->iq, doc, root);
	if (err != ERR_NONE) {
		fprintf(stderr, "Error getting IQ JSON: %s\n", getErrorDescription(err));
		return err;
	}

	return ERR_NONE;
}
