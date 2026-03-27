#ifndef INSTR_H
#define INSTR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "yyjson.h"
#include "error.h"
#include "register_table.h"
#include "active_list.h"
#include "pc.h"

typedef enum {
  ADD = 0,
  ADDI,
  SUB,
  MULU,
  DIVU,
  REMU,
  UNKNOWN_TYPE
} type_t;

typedef enum {
  OPERAND_REG,
  OPERAND_IMM
} operand_type_t;

typedef struct {
  operand_type_t type;
  union {
    struct {
      uint8_t log_id;
			uint8_t phy_id;
      reg_t value;
    } reg;
    int64_t imm;
  };
} operand_t;

#define MAX_TYPE_NAME_LENGTH 16
#define MAX_OPERAND_NAME_LENGTH 16

static const char* const type_names[] = {
  [ADD]  = "add",
  [ADDI] = "addi",
  [SUB]  = "sub",
  [MULU] = "mulu",
  [DIVU] = "divu",
  [REMU] = "remu"
};

typedef reg_t (*instruction_func_t)(operand_t, operand_t, bool*);

reg_t add_func(operand_t a, operand_t b, bool *is_exception);
reg_t addi_func(operand_t a, operand_t b, bool *is_exception);
reg_t sub_func(operand_t a, operand_t b, bool *is_exception);
reg_t mulu_func(operand_t a, operand_t b, bool *is_exception);
reg_t divu_func(operand_t a, operand_t b, bool *is_exception);
reg_t remu_func(operand_t a, operand_t b, bool *is_exception);

static instruction_func_t instruction_functions[] = {
  [ADD]  = add_func,
  [ADDI] = addi_func,
  [SUB]  = sub_func,
  [MULU] = mulu_func,
  [DIVU] = divu_func,
  [REMU] = remu_func
};

typedef struct {
  type_t type;
  char name[MAX_TYPE_NAME_LENGTH];
  operand_t dest;
  operand_t opA;
  operand_t opB;
  instruction_func_t function;
  bool is_exception;
  pc_t pc;
} instruction_t;

typedef struct {
  instruction_t *instructions;
  size_t count;
} program_t;

/**
 * @brief Initialize an instruction.
 */
int init_instruction(
  const char* name, operand_t dest, operand_t opA, operand_t opB, pc_t pc,
  instruction_t *instruction
);

/**
 * @brief Initialize the instruction type based on
 * its name.
 * Internal function.
 */
int get_instruction_type(const char *name, type_t *type);
int is_operand_ready(
	operand_t *operand, busy_bit_table_t *busy_bit_table, active_list_t *active_list,
	bool *is_ready
);

void print_instruction(const instruction_t *instruction);
void print_instruction_list(program_t *program);

yyjson_mut_val *instruction_get_json(yyjson_mut_doc *doc, void *instruction);

#endif // INSTR_H
