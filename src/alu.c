#include "alu.h"

int alu_init(alu_t *alu) {
	if (alu == NULL) {
		return ERR_NULL_PTR;
	}

	alu->stages = (alu_stages_t *) calloc(1, sizeof(alu_stages_t));
	if (alu->stages == NULL) {
		return ERR_MEM_ALLOC;
	}

	// mark all instructions as unknown in the beginning
	for (size_t i = 0; i < ALU_STAGES; i++) {
		alu->stages->instr_at_stage[i].type = UNKNOWN_TYPE;
	}

	alu->forwarding_path = &alu->stages->instr_at_stage[ALU_STAGES - 1];

	return ERR_NONE;
}

int alu_free(alu_t *alu) {
	if (alu == NULL) {
		return ERR_NULL_PTR;
	}

	free(alu->stages);

	return ERR_NONE;
}

int alu_flush(alu_t *alu) {
	if (alu == NULL) {
		return ERR_NULL_PTR;
	}

	for (size_t i = 0; i < ALU_STAGES; i++) {
		memset(
			&alu->stages->instr_at_stage[i],
			0,
			sizeof(instruction_t)
		);
		alu->stages->instr_at_stage[i].type = UNKNOWN_TYPE;
	}

	return ERR_NONE;
}

int alu_push_instruction(alu_t *alu, instruction_t instr) {
	if (alu == NULL) {
		return ERR_NULL_PTR;
	}

	size_t last_element = ALU_STAGES - 1;

	for (size_t i = last_element; i > 0; i--) {
		alu->stages->instr_at_stage[i] = alu->stages->instr_at_stage[i - 1];
	}

	alu->stages->instr_at_stage[0] = instr;

	return ERR_NONE;
}

int alu_cycle(alu_t *alu) {
	if (alu == NULL) {
		return ERR_NULL_PTR;
	}

	instruction_t *instr = alu->forwarding_path;

	// If invalid instruction, skip
	if (instr->type == UNKNOWN_TYPE) {
		return ERR_NONE;
	}

	// Execute the instruction
	instr->dest.reg.value =
		instr->function(
			instr->opA, instr->opB, &instr->is_exception
		);
	
	#ifdef DEBUG
		printf("ALU: result: %lu = %lu, %lu\n",
			instr->dest.reg.value,
			instr->opA.type == OPERAND_REG ? instr->opA.reg.value : instr->opA.imm,
			instr->opB.type == OPERAND_REG ? instr->opB.reg.value : instr->opB.imm
		);
	#endif

	return ERR_NONE;
}

void print_alu(alu_t *alu, size_t idx) {
	if (alu == NULL) {
			return;
	}

	printf("ALU %zu: [", idx);

	char instr_str[ALU_STAGES][64] = {0};

	for (size_t i = 0; i < ALU_STAGES; i++) {
			FILE *original_stdout = stdout;
			FILE *temp_file = tmpfile();
			if (temp_file == NULL) {
					printf("ERROR: Failed to create temporary file\n");
					return;
			}
			
			stdout = temp_file;
			
			if (alu->stages->instr_at_stage[i].type != UNKNOWN_TYPE) {
					print_instruction(&alu->stages->instr_at_stage[i]);
			} else {
					printf("[NO INSTR]");
			}
			
			fflush(stdout);
			
			stdout = original_stdout;
			
			rewind(temp_file);
			if (fgets(instr_str[i], sizeof(instr_str[i]), temp_file) == NULL) {
					strcpy(instr_str[i], "[NO INSTR]");
			}
			
			size_t len = strlen(instr_str[i]);
			if (len > 0 && instr_str[i][len-1] == '\n') {
					instr_str[i][len-1] = '\0';
			}
			
			fclose(temp_file);
	}
	
	const int width = 50;
	for (size_t i = 0; i < ALU_STAGES; i++) {
			size_t len = strlen(instr_str[i]);
			while (len > 0 && isspace(instr_str[i][len-1])) {
					instr_str[i][--len] = '\0';
			}

			printf("%-*s", width, instr_str[i]);
			if (i != ALU_STAGES - 1) {
					printf("|");
			}
	}
	printf("]\n");
}
