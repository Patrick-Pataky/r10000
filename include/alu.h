// ALU stage
#ifndef ALU_H
#define ALU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include "error.h"
#include "common.h"
#include "instruction.h"

#define ALU_STAGES 2

typedef struct {
	instruction_t instr_at_stage[ALU_STAGES]; // circular fifo queue
} alu_stages_t;

/**
 * Instruction at last stage is the value of the forwarding path.
 * 
 * Is updated periodically.
 */
typedef instruction_t* alu_forwarding_path_t;

typedef struct {
	alu_stages_t *stages;
	alu_forwarding_path_t forwarding_path;
} alu_t;

int alu_init(alu_t *alu);
int alu_free(alu_t *alu);
int alu_flush(alu_t *alu);

int alu_push_instruction(alu_t *alu, instruction_t instr);

/**
 * @brief Execute one cycle of the ALU.
 * 
 * Assumes that `alu_push_instruction` has been called before, to
 * fill the forwarding path.
 */
int alu_cycle(alu_t *alu);
void print_alu(alu_t *alu, size_t idx);

#endif // ALU_H
