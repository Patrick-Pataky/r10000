// Register File (physical & logical)
#ifndef REG_TABLE_H
#define REG_TABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "yyjson.h"
#include "error.h"
#include "common.h"

#define REG_LOG_FILE_SIZE 32
#define REG_LOG_FILE_MASK (REG_LOG_FILE_SIZE - 1)

#define REG_PHY_FILE_SIZE 64
#define REG_PHY_FILE_MASK (REG_PHY_FILE_SIZE - 1)

// Register File
typedef struct {
  reg_t physical[REG_PHY_FILE_SIZE];
} reg_file_t;

int reset_reg_file(reg_file_t *reg_file);

int reg_file_get_json(reg_file_t *reg_file, yyjson_mut_doc *doc, yyjson_mut_val *root);

// Register Map Table
typedef struct {
  uint8_t logical[REG_LOG_FILE_SIZE];
} reg_map_table_t;

int reset_reg_map_table(reg_map_table_t *reg_map_table);

int reg_map_table_get_json(
	reg_map_table_t *reg_map_table, yyjson_mut_doc *doc, yyjson_mut_val *root
);

// Busy bit table
typedef struct {
  bool is_busy[REG_PHY_FILE_SIZE];
} busy_bit_table_t;

int reset_busy_bit_table(busy_bit_table_t *busy_bit_table);

int busy_bit_table_get_json(
	busy_bit_table_t *busy_bit_table, yyjson_mut_doc *doc, yyjson_mut_val *root
);

yyjson_mut_val *reg_to_json(yyjson_mut_doc *doc, void *data);
yyjson_mut_val *reg_idx_to_json(yyjson_mut_doc *doc, void *data);

void print_reg_file(reg_file_t *reg_file);

#endif // REG_TABLE_H
