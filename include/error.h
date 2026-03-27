#ifndef ERROR_H
#define ERROR_H

typedef enum {
    ERR_NONE = 0,

    // Main
    ERR_ARGS_NUM = -127,
    ERR_FILE_PARSE,

    // PC
    ERR_PC_INIT,

    // Parser
    ERR_INVALID_FORMAT,
    ERR_INVALID_NAME,
    ERR_INVALID_JSON,

    // Data structures
    ERR_QUEUE_INIT,
    ERR_QUEUE_FULL,
    ERR_QUEUE_EMPTY,
    ERR_QUEUE_INVALID_INDEX,

		// ALU
		ERR_ALU_OPERATION,

    // All other errors
    ERR_NULL_PTR,
    ERR_BUFFER_OVERFLOW,
    ERR_MEM_ALLOC,
		ERR_INVALID_LOG_REG,
		ERR_INVALID_PHY_REG,
    ERR_UNKNOWN
} ErrorCode;

static inline const char *getErrorDescription(ErrorCode err) {
  switch (err) {
    case ERR_NONE:
      return "No error occurred.";

    // Main
    case ERR_ARGS_NUM:
      return "Invalid number of arguments provided.";
    case ERR_FILE_PARSE:
      return "Failed to parse the input file.";

    // PC
    case ERR_PC_INIT:
      return "Failed to initialize the program counter.";

    // Parser
    case ERR_INVALID_FORMAT:
      return "Invalid instruction format.";
    case ERR_INVALID_NAME:
      return "Invalid instruction name.";
    case ERR_INVALID_JSON:
      return "Invalid JSON format.";

    // Data structures
    case ERR_QUEUE_INIT:
      return "Failed to initialize the queue.";
    case ERR_QUEUE_FULL:
      return "Queue is full.";
    case ERR_QUEUE_EMPTY:
      return "Queue is empty.";
    case ERR_QUEUE_INVALID_INDEX:
      return "Invalid index for queue access.";
		
		// ALU
		case ERR_ALU_OPERATION:
			return "ALU operation failed.";

    // All other errors
    case ERR_NULL_PTR:
      return "Null pointer encountered.";
    case ERR_BUFFER_OVERFLOW:
      return "Buffer overflow occurred.";
    case ERR_MEM_ALLOC:
      return "Memory allocation failed.";
		case ERR_INVALID_LOG_REG:
			return "Invalid register number.";
		case ERR_INVALID_PHY_REG:
			return "Invalid register number.";
    case ERR_UNKNOWN:
    default:
      return "An unknown error occurred.";
  }
}

#endif /* ERROR_H */
