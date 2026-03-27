// Parse the instructions
#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "yyjson.h"
#include "instruction.h"
#include "register_table.h"
#include "pc.h"
#include "error.h"

/**
 * @brief Parses a register string into a register number.
 * 
 * Example: "x0" -> 0, "x1" -> 1, ..., "x31" -> 31
 */
int parse_operand(const char *op_str, operand_t *op);

/**
 * @brief Parses an instruction string into an instruction structure.
 * 
 * Example: "add x1, x2, x3" -> {ADD, "add", 1, 2, 3, &add_func}
 */
int parse_instruction(const char *instr_str, pc_t pc, instruction_t *instr);

/**
 * @brief Parses a JSON string from input_file into an array
 * of instructions, of size num_instructions.
 */
int parse_instructions(const char *input_file, instruction_t **instructions, size_t *num_instructions);

#endif // PARSER_H
