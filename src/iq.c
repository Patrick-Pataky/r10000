#include "iq.h"

const size_t IQ_CAPACITY = 32;

int iq_init(iq_t *iq) {
  if (iq == NULL) {
    return ERR_NULL_PTR;
  }

  iq->queue = (queue_t *) calloc(1, sizeof(queue_t));
  if (iq->queue == NULL) {
    return ERR_MEM_ALLOC;
  }

  int err = init_queue(iq->queue, IQ_CAPACITY, sizeof(iq_entry_t));
  if (err != ERR_NONE) {
    free(iq->queue);
    iq->queue = NULL;
    return err;
  }

  return ERR_NONE;
}

int iq_free(iq_t **iq) {
  if (iq == NULL || *iq == NULL) {
    return ERR_NULL_PTR;
  }

  int err = free_queue(&(*iq)->queue);
  if (err != ERR_NONE) {
    return err;
  }

  free(*iq);
  *iq = NULL;

  return ERR_NONE;
}

int iq_enqueue(iq_t *iq, instruction_t *instr, bool is_opA_ready, bool is_opB_ready) {
  if (iq == NULL || iq->queue == NULL) {
    return ERR_NULL_PTR;
  }

  iq_entry_t entry;
  entry.instr = *instr;
  entry.is_opA_ready = is_opA_ready;
  entry.is_opB_ready = is_opB_ready;

  int err = queue_push(iq->queue, &entry);
  if (err != ERR_NONE) {
    return err;
  }

  return ERR_NONE;
}

int iq_pop_logical(iq_t *iq, size_t index, iq_entry_t *entry) {
	if (iq == NULL || iq->queue == NULL || entry == NULL) {
		return ERR_NULL_PTR;
	}

	int err = queue_pop_logical(iq->queue, index, entry);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int iq_update_logical(iq_t *iq, size_t index, iq_entry_t *entry) {
	if (iq == NULL || iq->queue == NULL) {
		return ERR_NULL_PTR;
	}

	int err = queue_update_logical(iq->queue, index, entry);
	if (err != ERR_NONE) {
		return err;
	}

	return ERR_NONE;
}

int iq_get_json(iq_t *iq, yyjson_mut_doc *doc, yyjson_mut_val *root) {
	if (iq == NULL || iq->queue == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  yyjson_mut_val *iq_json = yyjson_mut_arr(doc);
	if (iq_json == NULL) {
		return ERR_MEM_ALLOC;
	}

  for (size_t i = 0; i < iq->queue->count; i++) {
    iq_entry_t entry;
    int err = queue_peek_logical(iq->queue, i, (void*) &entry);
    if (err != ERR_NONE) {
      fprintf(stderr, "Error peeking IQ entry: %s\n", getErrorDescription(err));
      return err;
    }

    yyjson_mut_val *entry_json = yyjson_mut_obj(doc);
    if (entry_json == NULL) {
      fprintf(stderr, "Error creating JSON object for IQ entry\n");
      return ERR_MEM_ALLOC;
    }

		yyjson_mut_obj_add_uint(doc, entry_json, "DestRegister", entry.instr.dest.reg.phy_id);
		yyjson_mut_obj_add_bool(doc, entry_json, "OpAIsReady", entry.is_opA_ready);

		// 0 is the default value for the tag
		const uint8_t DEFAULT_TAG = 0;

		uint8_t opA_tag = entry.is_opA_ready ? DEFAULT_TAG : entry.instr.opA.reg.phy_id;
		yyjson_mut_obj_add_uint(doc, entry_json, "OpARegTag", opA_tag);
		yyjson_mut_obj_add_uint(doc, entry_json, "OpAValue", entry.instr.opA.reg.value);
		yyjson_mut_obj_add_bool(doc, entry_json, "OpBIsReady", entry.is_opB_ready);

		uint8_t opB_tag = entry.is_opB_ready ? DEFAULT_TAG : entry.instr.opB.reg.phy_id;
		yyjson_mut_obj_add_uint(doc, entry_json, "OpBRegTag", opB_tag);

		if (entry.instr.opB.type == OPERAND_IMM) {
			yyjson_mut_obj_add_uint(doc, entry_json, "OpBValue", entry.instr.opB.imm);
		} else if (entry.instr.opB.type == OPERAND_REG) {
			yyjson_mut_obj_add_uint(doc, entry_json, "OpBValue", entry.instr.opB.reg.value);
		}

		// for some reason, "addi" is not expected, always "add"
		if (entry.instr.type == ADDI) {
			yyjson_mut_obj_add_str(doc, entry_json, "OpCode", "add");
		} else {
			// ! potential mem leak here
			char *opcode_name = malloc(MAX_TYPE_NAME_LENGTH * sizeof(char));
			strncpy(opcode_name, entry.instr.name, MAX_TYPE_NAME_LENGTH);
			opcode_name[MAX_TYPE_NAME_LENGTH - 1] = '\0';
			yyjson_mut_obj_add_str(doc, entry_json, "OpCode", opcode_name);
		}

		yyjson_mut_obj_add_uint(doc, entry_json, "PC", entry.instr.pc.current_pc);

		if (!yyjson_mut_arr_append(iq_json, entry_json)) {
			fprintf(stderr, "Error appending IQ entry JSON\n");
			return ERR_MEM_ALLOC;
		}
  }

  yyjson_mut_obj_add_val(doc, root, "IntegerQueue", iq_json);

  return ERR_NONE;
}

int iq_entry_is_ready(iq_entry_t *entry, bool *is_ready) {
	if (entry == NULL || is_ready == NULL) {
		return ERR_NULL_PTR;
	}

	*is_ready = entry->is_opA_ready && entry->is_opB_ready;

	return ERR_NONE;
}

int iq_refresh_ready(
	iq_t *iq, reg_file_t *reg_file, 
	reg_map_table_t *reg_map_table, busy_bit_table_t *busy_bit_table,
	active_list_t *active_list
)
{
	if (iq == NULL || iq->queue == NULL || reg_file == NULL
			|| reg_map_table == NULL || busy_bit_table == NULL || active_list == NULL) {
		return ERR_NULL_PTR;
	}

	size_t count = iq->queue->count;

	for (size_t i = 0; i < count; i++) {
		iq_entry_t entry;

		int err = queue_peek_logical(iq->queue, i, &entry);
		if (err != ERR_NONE) {
			return err;
		}

		bool is_opA_ready = false;
		bool is_opB_ready = false;

		err = is_operand_ready(
			&entry.instr.opA, busy_bit_table, active_list, &is_opA_ready
		);
		if (err != ERR_NONE) {
			return err;
		}

		err = is_operand_ready(
			&entry.instr.opB, busy_bit_table, active_list, &is_opB_ready
		);
		if (err != ERR_NONE) {
			return err;
		}

		#ifdef DEBUG
			if (entry.instr.pc.current_pc == 12) {
				printf("[IQ]: Entry %zu: OpA: %s, OpB: %s\n",
					i,
					is_opA_ready ? "Ready" : "Not Ready",
					is_opB_ready ? "Ready" : "Not Ready"
				);
				print_iq_entry(&entry);
			}
		#endif

		entry.is_opA_ready = is_opA_ready;
		entry.is_opB_ready = is_opB_ready;

		if (is_opA_ready && entry.instr.opA.type == OPERAND_REG) {
			entry.instr.opA.reg.value = reg_file->physical[entry.instr.opA.reg.phy_id];
		}

		if (is_opB_ready && entry.instr.opB.type == OPERAND_REG) {
			entry.instr.opB.reg.value = reg_file->physical[entry.instr.opB.reg.phy_id];
		}

		err = iq_update_logical(iq, i, &entry);
		if (err != ERR_NONE) {
			return err;
		}
	}

	return ERR_NONE;
}

int iq_get_first_n_ready(iq_t *iq, size_t n, iq_entry_t *entries, size_t *num_ready) {
	if (iq == NULL || iq->queue == NULL || entries == NULL || num_ready == NULL) {
		return ERR_NULL_PTR;
	}

	size_t count = 0;

	// iterate through the queue and find the first n ready entries
	for (size_t i = 0; i < iq->queue->count && count < n;) {
		iq_entry_t entry;

		int err = queue_peek_logical(iq->queue, i, &entry);
		if (err != ERR_NONE) {
			return err;
		}

		if (entry.instr.is_exception) {
			break;
		}

		bool is_ready;
		err = iq_entry_is_ready(&entry, &is_ready);
		if (err != ERR_NONE) {
			return err;
		}

		if (is_ready) {
			entries[count++] = entry;

			// delete the entry from the queue
			iq_entry_t dummy;

			err = iq_pop_logical(iq, i, &dummy);
			if (err != ERR_NONE) {
				return err;
			}
		} else {
			// if we didn't remove the entry, move to the next one
			i++;
		}
	}

	*num_ready = count;

	return ERR_NONE;
}

int iq_set_fw(iq_t *iq, alu_forwarding_path_t *forwarding_path) {
	if (iq == NULL || iq->queue == NULL || forwarding_path == NULL) {
		return ERR_NULL_PTR;
	}

	instruction_t *instr = *forwarding_path;

	for (size_t i = 0; i < iq->queue->count; i++) {
		iq_entry_t entry;

		int err = queue_peek_logical(iq->queue, i, &entry);
		if (err != ERR_NONE) {
			return err;
		}

		bool is_modified = false;

		if (instr->dest.reg.phy_id == entry.instr.opA.reg.phy_id) {
			entry.is_opA_ready = true;
			entry.instr.opA.reg.value = instr->dest.reg.value;

			is_modified = true;
		}

		if (instr->dest.reg.phy_id == entry.instr.opB.reg.phy_id) {
			entry.is_opB_ready = true;
			entry.instr.opB.reg.value = instr->dest.reg.value;

			is_modified = true;
		}

		if (is_modified) {
			// update the entry in the queue
			int err = iq_update_logical(iq, i, &entry);
			if (err != ERR_NONE) {
				return err;
			}
		}
	}

	return ERR_NONE;
}

void print_iq_entry(iq_entry_t *entry) {
	if (entry == NULL) {
		printf("[IQ]: Entry is NULL\n");
		return;
	}

	printf("*************************************************\n");
	printf("[IQ]: Entry:\n");
	printf("  Instruction: "); print_instruction(&entry->instr); printf("\n");
	printf("  OpA: ");
	if (entry->is_opA_ready) {
		printf("Ready, %lu\n", entry->instr.opA.type == OPERAND_REG ? entry->instr.opA.reg.value : entry->instr.opA.imm);
	} else {
		printf("Not Ready, p%u\n", entry->instr.opA.reg.phy_id);
	}
	printf("  OpB: ");
	if (entry->is_opB_ready) {
		printf("Ready, %lu\n", entry->instr.opB.type == OPERAND_REG ? entry->instr.opB.reg.value : entry->instr.opB.imm);
	} else {
		printf("Not Ready, p%u\n", entry->instr.opB.reg.phy_id);
	}
	printf("  Dest: x%u -> p%u\n", entry->instr.dest.reg.log_id, entry->instr.dest.reg.phy_id);
	printf("  PC: %lu\n", entry->instr.pc.current_pc);
	printf("  OpCode: %s\n", entry->instr.name);
	printf("*************************************************\n");
}
