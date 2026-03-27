#ifndef PROC_H
#define PROC_H

#include <stdlib.h>
#include <stdint.h>
#include "yyjson.h"
#include "error.h"
#include "data_structures.h"
#include "pc.h"
#include "register_table.h"
#include "fetch_decode.h"
#include "rename_dispatch.h"
#include "issue.h"

typedef struct {
  pc_t pc;
  program_t program;
  bool finished;
  exception_status_t exception_status;

  reg_file_t reg_file;
  reg_map_table_t reg_map_table;
  free_list_t free_list;
  busy_bit_table_t busy_bit_table;
  active_list_t active_list;
} processor_state_t;

typedef struct {
	// Core processor state
	processor_state_t state;
	
	// Pipeline stages
	fetch_decode_t *fetch_decode;
	decoded_inst_reg_t *dir;
	rename_dispatch_t *rename_dispatch;
	iq_t *iq;
	issue_t *issue;
	commit_t *commit;
	
	// Execution units
	alu_t *alus[NUM_ALUS];
	
	// Forwarding paths
	alu_forwarding_path_t *fw_paths;
} processor_t;

/**
 * @brief Initialize the processor with the given program
 * 
 * This function:
 * 
 * 1. Initializes all processor state components (register file, tables, etc.)
 * 
 * 2. Creates and initializes all pipeline stages
 * 
 * 3. Links the pipeline stages together
 * 
 * 4. Sets up the program to be executed
 */
int init_processor(processor_t *processor, program_t *program);

int free_processor(processor_t **processor);

/**
 * @brief Simulates a single cycle of the processor
 */
void processor_cycle(processor_t* processor);

/**
 * @brief Main simulation loop
 * 
 * Initializes the processor state, runs cycles until the end,
 * and collects the history of the simulation.
 */
void run_simulation(processor_t *processor, yyjson_mut_doc *doc, yyjson_mut_val *root);

int processor_get_json(processor_t *processor, yyjson_mut_doc *doc, yyjson_mut_val *root);

#endif // PROC_H
