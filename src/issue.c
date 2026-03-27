#include "issue.h"

int init_issue(issue_t *issue, iq_t *iq, alu_t **alus, 
	reg_file_t *reg_file, reg_map_table_t *reg_map_table,
	busy_bit_table_t *busy_bit_table, active_list_t *active_list) {
	if (issue == NULL || iq == NULL || alus == NULL || alus[0] == NULL
			|| reg_file == NULL || reg_map_table == NULL || busy_bit_table == NULL
			|| active_list == NULL) {
		return ERR_NULL_PTR;
	}

	issue->iq = iq;
	issue->reg_file = reg_file;
	issue->reg_map_table = reg_map_table;
	issue->busy_bit_table = busy_bit_table;
	issue->active_list = active_list;

	for (int i = 0; i < NUM_ALUS; i++) {
		if (alus[i] == NULL) {
			return ERR_NULL_PTR;
		}

		issue->alus[i] = alus[i];
	}

	return ERR_NONE;
}

int free_issue(issue_t **issue) {
	if (issue == NULL || *issue == NULL) {
		return ERR_NULL_PTR;
	}

	free(*issue);
	*issue = NULL;

	return ERR_NONE;
}

int issue_cycle(issue_t *issue) {
	if (issue == NULL) {
		return ERR_NULL_PTR;
	}

	// Scan integer queue and pick at most 4 instructions
	// to issue to the ALUs
	size_t num_ready = 0;
	iq_entry_t entries[NUM_ALUS];

	int err = iq_refresh_ready(
		issue->iq, issue->reg_file, 
		issue->reg_map_table, issue->busy_bit_table,
		issue->active_list
	);
	if (err != ERR_NONE) {
		return err;
	}

	err = iq_get_first_n_ready(issue->iq, NUM_ALUS, entries, &num_ready);
	if (err != ERR_NONE) {
		return err;
	}

	for (size_t i = 0; i < NUM_ALUS; i++) {
		// Issue the instruction to the ALU
		instruction_t instr = {0};

		if (i >= num_ready) {
			instr.type = UNKNOWN_TYPE;
		} else {
			instr = entries[i].instr;
		}

		err = alu_push_instruction(issue->alus[i], instr);
		if (err != ERR_NONE) {
			return err;
		}

		#ifdef DEBUG
			printf("Issue stage: Issued instruction [");
			print_instruction(&instr);
			printf("] to ALU %zu\n", i);
		#endif
	}

	return num_ready;
}
