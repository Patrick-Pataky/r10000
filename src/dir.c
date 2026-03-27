#include "dir.h"

int init_decoded_inst_reg(decoded_inst_reg_t *decoded_inst_reg) {
  if (decoded_inst_reg == NULL) {
    return ERR_NULL_PTR;
  }

	decoded_inst_reg->queue = (queue_t*) calloc(1, sizeof(queue_t));
	if (decoded_inst_reg->queue == NULL) {
		return ERR_MEM_ALLOC;
	}

  int err = init_queue(
		decoded_inst_reg->queue,
		INSTRUCTIONS_PER_CYCLE,
		sizeof(instruction_t)
	);

	if (err != ERR_NONE) {
		free(decoded_inst_reg->queue);
		decoded_inst_reg->queue = NULL;
	}

  return err;
}

int free_decoded_inst_reg(decoded_inst_reg_t **decoded_inst_reg) {
  if (decoded_inst_reg == NULL || *decoded_inst_reg == NULL) {
    return ERR_NULL_PTR;
  }

  queue_t *queue = (*decoded_inst_reg)->queue;

  if (queue != NULL) {
    if (queue->data != NULL) {
      free(queue->data);
      queue->data = NULL;
    }
    free(queue);
    (*decoded_inst_reg)->queue = NULL;
  }

  free(*decoded_inst_reg);
  *decoded_inst_reg = NULL;

  return ERR_NONE;
}

int decoded_inst_reg_get_json(
	decoded_inst_reg_t *decoded_inst_reg, yyjson_mut_doc *doc, yyjson_mut_val *root
) {
  if (decoded_inst_reg == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  int res = queue_get_json(
    decoded_inst_reg->queue,
    doc, root,
    "DecodedPCs",
    instruction_get_json
  );

  if (res != ERR_NONE) {
    fprintf(stderr, "[DIR]: Failed to get JSON for decoded instruction register\n");
    return res;
  }

  return ERR_NONE;
}
