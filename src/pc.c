#include "pc.h"

const uint64_t PC_INIT      = 0x0;
const uint64_t PC_EXCEPTION = 0x10000;

int pc_init(pc_t *pc) {
  if (pc == NULL) {
    return ERR_NULL_PTR;
  }

  pc->current_pc = PC_INIT;

  return ERR_NONE;
}

int pc_set(pc_t *old_pc, pc_t *new_pc) {
  if (old_pc == NULL || new_pc == NULL) {
    return ERR_NULL_PTR;
  }

  old_pc->current_pc = new_pc->current_pc;

  return ERR_NONE;
}

int pc_get(pc_t *pc, uint64_t *value) {
  if (pc == NULL || value == NULL) {
    return ERR_NULL_PTR;
  }

  *value = pc->current_pc;

  return ERR_NONE;
}

int pc_inc(pc_t *pc) {
  if (pc == NULL) {
    return ERR_NULL_PTR;
  }

  pc->current_pc += INSTRUCTIONS_PER_CYCLE;

  return ERR_NONE;
}

int pc_set_exception(pc_t *pc) {
  if (pc == NULL) {
    return ERR_NULL_PTR;
  }

  pc->current_pc = PC_EXCEPTION;

  return ERR_NONE;
}

int pc_is_exception(pc_t *pc, bool *res) {
  if (pc == NULL || res == NULL) {
    return ERR_NULL_PTR;
  }
  
  *res = (pc->current_pc == PC_EXCEPTION);
  return ERR_NONE;
}

int pc_get_json(pc_t *pc, yyjson_mut_doc *doc, yyjson_mut_val *root) {
  if (pc == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  if (!yyjson_mut_obj_add_uint(doc, root, "PC", pc->current_pc)) {
		fprintf(stderr, "Error adding PC to JSON\n");
    return ERR_INVALID_JSON;
  }

  return ERR_NONE;
}
