#include "fetch_decode.h"

int init_fetch_decode(fetch_decode_t *fetch_decode, 
                     pc_t *pc,
                     decoded_inst_reg_t *dir,
                     program_t *program) {
  if (fetch_decode == NULL || pc == NULL || dir == NULL || program == NULL) {
    return ERR_NULL_PTR;
  }

  fetch_decode->pc = pc;
  fetch_decode->dir = dir;
  fetch_decode->program = program;
  fetch_decode->exception_detected = false;
	fetch_decode->backpressure = false;

  // Initialize PC to 0
  pc_init(pc);

  return ERR_NONE;
}

int free_fetch_decode(fetch_decode_t **fetch_decode) {
	if (fetch_decode == NULL || *fetch_decode == NULL) {
		return ERR_NULL_PTR;
	}

	// Don't free pc, dir, or program as they're owned by the caller
	free(*fetch_decode);
	*fetch_decode = NULL;

	return ERR_NONE;
}

int fetch_decode_cycle(fetch_decode_t *fetch_decode) {
	if (fetch_decode == NULL) {
		return ERR_NULL_PTR;
	}

	// If there's an exception detected, set PC to exception address
	// regardless of backpressure
	if (fetch_decode->exception_detected) {
		pc_set_exception(fetch_decode->pc);

		#ifdef DEBUG
			printf("F&D: Exception detected, setting PC to exception address\n");
		#endif

		return 0;
	}

	// If rename/dispatch applies backpressure, don't fetch any instructions
	if (fetch_decode->backpressure) {
		#ifdef DEBUG
			printf("F&D: Backpressure detected, not fetching instructions\n");
		#endif

		return 0;
	}

	// Get current PC value
	uint64_t current_pc;
	int res = pc_get(fetch_decode->pc, &current_pc);
	if (res != ERR_NONE) {
		return res;
	}

	// Calculate how many instructions to fetch
	size_t max_instructions = INSTRUCTIONS_PER_CYCLE;
	size_t remaining_instructions = 0;

	if (current_pc < fetch_decode->program->count) {
		remaining_instructions = fetch_decode->program->count - current_pc;
	}

	size_t instructions_to_fetch = MIN(max_instructions, remaining_instructions);

	// If no instructions to fetch, we're done
	if (instructions_to_fetch == 0) {
		#ifdef DEBUG
			printf("F&D: No instructions to fetch, PC: %lu\n", current_pc);
		#endif

		return 0;
	}

	// Fetch and decode instructions
	for (size_t i = 0; i < instructions_to_fetch; i++) {
		instruction_t *instr = &fetch_decode->program->instructions[current_pc + i];
		
		// Add to decoded instruction register
		res = queue_push(fetch_decode->dir->queue, (void*) instr);
		if (res != ERR_NONE) {
			if (res == ERR_QUEUE_FULL) {
				instructions_to_fetch = i;
				break;
			} else {
				fprintf(stderr, "Error pushing instruction to DIR: %d\n", res);
				print_instruction(instr);
				return res;
			}
		}

		#ifdef DEBUG
			printf("F&D: Pushed instruction");
			print_instruction(instr);
			printf(" to DIR\n");
		#endif
	}

	// Increment PC by number of instructions fetched
	if (instructions_to_fetch > 0) {
		pc_t new_pc;
		new_pc.current_pc = current_pc + instructions_to_fetch;
		
		res = pc_set(fetch_decode->pc, &new_pc);
		if (res != ERR_NONE) {
			return res;
		}
	}

	return instructions_to_fetch;
}

int signal_exception(fetch_decode_t *fetch_decode) {
	if (fetch_decode == NULL) {
		return ERR_NULL_PTR;
	}

	fetch_decode->exception_detected = true;
	return ERR_NONE;
}
