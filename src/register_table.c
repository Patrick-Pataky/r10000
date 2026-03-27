#include "register_table.h"

int reset_reg_file(reg_file_t *reg_file) {
  if (reg_file == NULL) {
    return ERR_NULL_PTR;
  }

  for (size_t i = 0; i < REG_PHY_FILE_SIZE; i++) {
    reg_file->physical[i] = 0;
  }

  return ERR_NONE;
}

int reg_file_get_json(reg_file_t *reg_file, yyjson_mut_doc *doc, yyjson_mut_val *root) {
  if (reg_file == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  yyjson_mut_val *reg_array = yyjson_mut_arr_with_uint64(doc, reg_file->physical, REG_PHY_FILE_SIZE);
	if (reg_array == NULL) {
		fprintf(stderr, "[RF]: Failed to create JSON array\n");
		return ERR_MEM_ALLOC;
	}

  if (!yyjson_mut_obj_add_val(doc, root, "PhysicalRegisterFile", reg_array)) {
		fprintf(stderr, "[RF]: Failed to set JSON object\n");
		return ERR_MEM_ALLOC;
	}

  return ERR_NONE;
}

int reset_reg_map_table(reg_map_table_t *reg_map_table) {
  if (reg_map_table == NULL) {
    return ERR_NULL_PTR;
  }

  for (size_t i = 0; i < REG_LOG_FILE_SIZE; i++) {
    reg_map_table->logical[i] = i;
  }

  return ERR_NONE;
}

int reg_map_table_get_json(
	reg_map_table_t *reg_map_table, yyjson_mut_doc *doc, yyjson_mut_val *root
) {
  if (reg_map_table == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  yyjson_mut_val *reg_array = yyjson_mut_arr_with_uint8(
		doc, reg_map_table->logical, REG_LOG_FILE_SIZE
	);
	if (reg_array == NULL) {
		fprintf(stderr, "[RMT]: Failed to create JSON array\n");
		return ERR_MEM_ALLOC;
	}

	if (!yyjson_mut_obj_add_val(doc, root, "RegisterMapTable", reg_array)) {
		fprintf(stderr, "[RMT]: Failed to set JSON object\n");
		return ERR_MEM_ALLOC;
	}

  return ERR_NONE;
}

int reset_busy_bit_table(busy_bit_table_t *busy_bit_table) {
  if (busy_bit_table == NULL) {
    return ERR_NULL_PTR;
  }

  for (size_t i = 0; i < REG_PHY_FILE_SIZE; i++) {
    busy_bit_table->is_busy[i] = false;
  }

  return ERR_NONE;
}

int busy_bit_table_get_json(
	busy_bit_table_t *busy_bit_table, yyjson_mut_doc *doc, yyjson_mut_val *root
) {
  if (busy_bit_table == NULL || doc == NULL || root == NULL) {
    return ERR_NULL_PTR;
  }

  yyjson_mut_val *busy_array = yyjson_mut_arr_with_bool(
		doc, busy_bit_table->is_busy, REG_PHY_FILE_SIZE
	);
	if (busy_array == NULL) {
		fprintf(stderr, "[BBT]: Failed to create JSON array\n");
		return ERR_MEM_ALLOC;
	}

	if (!yyjson_mut_obj_add_val(doc, root, "BusyBitTable", busy_array)) {
		fprintf(stderr, "[BBT]: Failed to set JSON object\n");
		return ERR_MEM_ALLOC;
	}

  return ERR_NONE;
}

yyjson_mut_val *reg_to_json(yyjson_mut_doc *doc, void *data) {
  if (data == NULL || doc == NULL) {
    return NULL;
  }

	reg_t value = *(reg_t *)data;
	yyjson_mut_val *val = yyjson_mut_uint(doc, value);
  if (val == NULL) {
    fprintf(stderr, "[REG]: Failed to create JSON integer\n");
  }

  return val;
}

yyjson_mut_val *reg_idx_to_json(yyjson_mut_doc *doc, void *data) {
  if (data == NULL) {
    return NULL;
  }

	uint8_t idx = *(uint8_t *)data;
	yyjson_mut_val *val = yyjson_mut_uint(doc, idx);
  if (val == NULL) {
    fprintf(stderr, "[REG]: Failed to create JSON integer\n");
  }

  return val;
}

void print_reg_file(reg_file_t *reg_file) {
	if (reg_file == NULL) {
		printf("[RF]: NULL register file\n");
		return;
	}

	printf("[RF]: Physical Register File:\n");
	for (size_t i = 0; i < REG_PHY_FILE_SIZE; i++) {
		printf("R%02zu: %21lu", i, reg_file->physical[i]);
		if (i % 4 == 3) {
			printf("\n");
		} else {
			printf(" | ");
		}
	}
	printf("[RF] ---------------------------------\n");
}
