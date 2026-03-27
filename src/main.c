#include <stdlib.h>
#include <stdio.h>

#include "processor.h"
#include "error.h"
#include "parser.h"
#include "data_structures.h"
#include "yyjson.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <path_to_input.json> <path_to_output.json>\n", argv[0]);
    
    return ERR_ARGS_NUM;
  }

  int res;

  // 0. parse JSON to get the program
  instruction_t *instructions = NULL;
  size_t num_instructions = 0;

  res = parse_instructions(argv[1], &instructions, &num_instructions);

  if (res != ERR_NONE) {
    fprintf(stderr, "Error parsing instructions: %s\n", getErrorDescription(res));
    
    return res;
  }

	program_t program = {0};
	program.instructions = instructions;
	program.count = num_instructions;

	#ifdef DEBUG
		printf("Parsed %zu instructions from %s\n", num_instructions, argv[1]);

		printf("Instruction List:\n");
		print_instruction_list(&program);
	#endif

  // 1. Initialize the processor
	processor_t *processor = (processor_t *) calloc(1, sizeof(processor_t));

	if (processor == NULL) {
		fprintf(stderr, "Error allocating memory for processor\n");
		return ERR_MEM_ALLOC;
	}

	res = init_processor(processor, &program);
	if (res != ERR_NONE) {
		fprintf(stderr, "Error initializing processor: %s\n", getErrorDescription(res));
		free_processor(&processor);
		return res;
	}

	// 2. Run the simulation
	yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
  yyjson_mut_val *root = yyjson_mut_arr(doc);
  yyjson_mut_doc_set_root(doc, root);

	run_simulation(processor, doc, root);

	// 3. Write JSON with formatting
	yyjson_write_err err;

  if (!yyjson_mut_write_file(
				argv[2], doc, 
				YYJSON_WRITE_PRETTY_TWO_SPACES | YYJSON_WRITE_ESCAPE_UNICODE, 
				NULL, &err)) {
		fprintf(stderr, "Error writing JSON to file: %s\n", err.msg);
		yyjson_mut_doc_free(doc);
		free_processor(&processor);
		return ERR_FILE_PARSE;
  }

	yyjson_mut_doc_free(doc);

	// 4. Free the processor & all resources
	res = free_processor(&processor);

	if (res != ERR_NONE) {
		fprintf(stderr, "Error freeing processor: %s\n", getErrorDescription(res));
		return res;
	}

	free(instructions);
	instructions = NULL;

  return ERR_NONE;
}
