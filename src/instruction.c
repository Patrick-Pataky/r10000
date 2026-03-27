#include "instruction.h"

reg_t add_func(operand_t a, operand_t b, bool *is_exception) {
  *is_exception = false;

	assert(a.type == OPERAND_REG);
	assert(b.type == OPERAND_REG);

	sreg_t res = (sreg_t) a.reg.value + (sreg_t) b.reg.value;
  return (ureg_t) res;
}

reg_t addi_func(operand_t a, operand_t b, bool *is_exception) {
  *is_exception = false;

	assert(a.type == OPERAND_REG);
	assert(b.type == OPERAND_IMM);

	sreg_t res = (sreg_t) a.reg.value + (sreg_t) b.imm;
  return (ureg_t) res;
}

reg_t sub_func(operand_t a, operand_t b, bool *is_exception) {
  *is_exception = false;

	assert(a.type == OPERAND_REG);
	assert(b.type == OPERAND_REG);

	sreg_t res = (sreg_t) a.reg.value - (sreg_t) b.reg.value;
  return (ureg_t) res;
}

reg_t mulu_func(operand_t a, operand_t b, bool *is_exception) {
  *is_exception = false;

	assert(a.type == OPERAND_REG);
	assert(b.type == OPERAND_REG);

	ureg_t res = (ureg_t) a.reg.value * (ureg_t) b.reg.value;
  return res;
}

reg_t divu_func(operand_t a, operand_t b, bool *is_exception) {
	assert(a.type == OPERAND_REG);
	assert(b.type == OPERAND_REG);

  if (b.reg.value == 0) {
    *is_exception = true;
    return 0;
  }

  *is_exception = false;

	ureg_t res = (ureg_t) a.reg.value / (ureg_t) b.reg.value;
  return res;
}

reg_t remu_func(operand_t a, operand_t b, bool *is_exception) {
	assert(a.type == OPERAND_REG);
	assert(b.type == OPERAND_REG);

  if (b.reg.value == 0) {
    *is_exception = true;
    return 0;
  }

  *is_exception = false;

	ureg_t res = (ureg_t) a.reg.value % (ureg_t) b.reg.value;
  return res;
}

int init_instruction(
  const char* name, operand_t dest, operand_t opA, operand_t opB, 
  pc_t pc, instruction_t *instruction
) {
  if (name == NULL || instruction == NULL) {
    return ERR_NULL_PTR;
  }

  int res = get_instruction_type(name, &instruction->type);

  if (res != ERR_NONE) {
    return res;
  }

  strncpy(instruction->name, name, MAX_TYPE_NAME_LENGTH - 1);
  instruction->name[MAX_TYPE_NAME_LENGTH - 1] = '\0';

  instruction->dest = dest;
  instruction->opA = opA;
  instruction->opB = opB;
  instruction->function = instruction_functions[instruction->type];
  instruction->is_exception = false;
  instruction->pc.current_pc = pc.current_pc;

  return ERR_NONE;
}

int get_instruction_type(const char *name, type_t *type) {
  if (name == NULL || type == NULL) {
    return ERR_NULL_PTR;
  }

  for (size_t i = 0; i < (sizeof(type_names) / sizeof(type_names[0])); i++) {
    if (strcmp(name, type_names[i]) == 0 && i != UNKNOWN_TYPE) {
      *type = i;

      return ERR_NONE;
    }
  }

  return ERR_INVALID_NAME;
}

int is_operand_ready(
	operand_t *operand, busy_bit_table_t *busy_bit_table, active_list_t *active_list,
	bool *is_ready
) {
	if (operand == NULL || busy_bit_table == NULL || is_ready == NULL) {
		return ERR_NULL_PTR;
	}

	if (operand->type == OPERAND_IMM) {
		*is_ready = true;
	} else if (operand->type == OPERAND_REG) {
		bool tmp_is_ready = !busy_bit_table->is_busy[operand->reg.phy_id];

		if (tmp_is_ready) {
			// is not busy, must check if not in exception in
			// the active list
			size_t count = active_list->list->count;
			for (size_t i = 0; i < count; i++) {
				active_list_entry_t entry;
				int err = queue_peek_logical(active_list->list, i, &entry);
				if (err != ERR_NONE) {
					return err;
				}

				// If this entry has the same logical register as destination and has an exception
				if (entry.curr_phy_dest == operand->reg.phy_id && entry.is_exception) {
					tmp_is_ready = false;
					break;
				}
			}
		}

		*is_ready = tmp_is_ready;
	}

	return ERR_NONE;
}

void print_instruction(const instruction_t *instruction) {
  if (instruction == NULL || instruction->type == UNKNOWN_TYPE) {
    printf("[NO INSTR]");
		return;
  }

  printf("[%3ld]: %s x%hhu [p%hhu], ", 
    instruction->pc.current_pc, instruction->name, 
		instruction->dest.reg.log_id, instruction->dest.reg.phy_id
  );

  if (instruction->opA.type == OPERAND_REG) {
    printf("x%hhu [p%hhu], ", instruction->opA.reg.log_id, instruction->opA.reg.phy_id);
  } else {
    printf("%ld, ", instruction->opA.imm);
  }

  if (instruction->opB.type == OPERAND_REG) {
    printf("x%hhu [p%hhu]", instruction->opB.reg.log_id, instruction->opB.reg.phy_id);
  } else {
    printf("%ld", instruction->opB.imm);
  }

	// printf("  (%lu) = (%lu),(%lu);",
	// 	instruction->dest.reg.value,
	// 	instruction->opA.type == OPERAND_REG ? instruction->opA.reg.value : instruction->opA.imm,
	// 	instruction->opB.type == OPERAND_REG ? instruction->opB.reg.value : instruction->opB.imm
	// );

	if (instruction->is_exception) {
		printf(" " RED "[EXC]" RESET);
	}
}

void print_instruction_list(program_t *program) {
  if (program == NULL || program->count == 0 || program->instructions == NULL) {
		printf("[Instruction List]: NULL program\n");
    return;
  }

  size_t size = program->count;
  instruction_t *instruction_list = program->instructions;

  for (size_t i = 0; i < size; i++) {
    print_instruction(&instruction_list[i]);
		printf("\n");
  }
}

yyjson_mut_val *instruction_get_json(yyjson_mut_doc *doc, void *instruction) {
  if (instruction == NULL || doc == NULL) {
    return NULL;
  }

  instruction_t *inst = (instruction_t *) instruction;

	return yyjson_mut_uint(doc, inst->pc.current_pc);
}
