#include "parser.h"

int parse_operand(const char *op_str, operand_t *op) {
  if (op_str == NULL || op == NULL) {
    return ERR_NULL_PTR;
  }

  reg_t res;

  if (op_str[0] != 'x') {
    res = (reg_t) atoi(op_str);

    op->type = OPERAND_IMM;
    op->imm = res;
  } else {
    res = (reg_t) atoi(op_str + 1);
    
    if (res < 0 || res >= REG_LOG_FILE_SIZE) {
      return ERR_INVALID_LOG_REG;
    }

    op->type = OPERAND_REG;
    op->reg.log_id = (uint8_t) res;
		op->reg.phy_id = 0;
		op->reg.value = 0;
  }

  return ERR_NONE;
}

int parse_instruction(const char *instr_str, pc_t pc, instruction_t *instr) {
  if (instr_str == NULL || instr == NULL) {
    return ERR_NULL_PTR;
  }

  instruction_t *inst = instr;
  inst->pc.current_pc = pc.current_pc;

  char name[MAX_TYPE_NAME_LENGTH];

  char dest[MAX_OPERAND_NAME_LENGTH];
  char opA[MAX_OPERAND_NAME_LENGTH];
  char opB[MAX_OPERAND_NAME_LENGTH];

  int count = sscanf(instr_str, "%s %[^,], %[^,], %s", name, dest, opA, opB);
  if (count != 4) {
    return ERR_INVALID_FORMAT;
  }

  strncpy(inst->name, name, MAX_TYPE_NAME_LENGTH - 1);
  inst->name[MAX_TYPE_NAME_LENGTH - 1] = '\0';

  int res;

  res = get_instruction_type(name, &inst->type);
  if (res != ERR_NONE) {
    return res;
  }

  res = parse_operand(dest, &inst->dest);
  if (res != ERR_NONE) {
    return res;
  }

  res = parse_operand(opA, &inst->opA);
  if (res != ERR_NONE) {
    return res;
  }

  res = parse_operand(opB, &inst->opB);
  if (res != ERR_NONE) {
    return res;
  }

  inst->function = instruction_functions[inst->type];

  return ERR_NONE;
}

int parse_instructions(const char *input_file, instruction_t **instructions, size_t *num_instructions) {
  if (input_file == NULL || instructions == NULL || num_instructions == NULL) {
    return ERR_NULL_PTR;
  }

  // Load JSON file
  yyjson_doc *doc = yyjson_read_file(input_file, 0, NULL, NULL);
  if (doc == NULL) {
    fprintf(stderr, "Error loading JSON file\n");
    return ERR_FILE_PARSE;
  }

  // Get the root value
  yyjson_val *root = yyjson_doc_get_root(doc);
  if (!yyjson_is_arr(root)) {
    fprintf(stderr, "Error: JSON root is not an array.\n");
    yyjson_doc_free(doc);
    return ERR_INVALID_FORMAT;
  }

  // Get array size and allocate memory
  size_t size = yyjson_arr_size(root);
  *num_instructions = size;

  *instructions = calloc(size, sizeof(instruction_t));
  if (*instructions == NULL) {
    fprintf(stderr, "Error allocating memory for instructions.\n");
    yyjson_doc_free(doc);
    return ERR_MEM_ALLOC;
  }

  // Parse each instruction
  yyjson_arr_iter iter;
  yyjson_arr_iter_init(root, &iter);
  yyjson_val *item;
  size_t idx = 0;
  
  while ((item = yyjson_arr_iter_next(&iter))) {
    if (yyjson_is_str(item)) {
      const char *instr_str = yyjson_get_str(item);

      pc_t pc = { .current_pc = idx };
      int res = parse_instruction(instr_str, pc, &(*instructions)[idx]);

      if (res != ERR_NONE) {
        fprintf(stderr, "Error parsing instruction at index %zu: %s\n", 
                idx, getErrorDescription(res));
        
        free(*instructions);
        yyjson_doc_free(doc);
        return res;
      }
    } else {
      fprintf(stderr, "Error: JSON item at index %zu is not a string.\n", idx);
      free(*instructions);
      yyjson_doc_free(doc);
      return ERR_INVALID_FORMAT;
    }
    idx++;
  }

  // Free the JSON document
  yyjson_doc_free(doc);
  return ERR_NONE;
}
