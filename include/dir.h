// Decoded Instruction Register
#ifndef DIR_H
#define DIR_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "yyjson.h"
#include "data_structures.h"
#include "instruction.h"
#include "error.h"

// Decoded Instruction Register
// (decoded but not yet renamed nor dispatched)
typedef struct {
  queue_t *queue;
} decoded_inst_reg_t;

int init_decoded_inst_reg(decoded_inst_reg_t *decoded_inst_reg);
int free_decoded_inst_reg(decoded_inst_reg_t **decoded_inst_reg);

int decoded_inst_reg_get_json(
	decoded_inst_reg_t *decoded_inst_reg, yyjson_mut_doc *doc, yyjson_mut_val *root
);

#endif // DIR_H
