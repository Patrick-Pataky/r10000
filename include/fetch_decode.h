// Fetch & Decode stage
#ifndef FETCH_DECODE_H
#define FETCH_DECODE_H

#include <stdbool.h>
#include "pc.h"
#include "dir.h"
#include "instruction.h"
#include "error.h"
#include "common.h"

typedef struct {
    pc_t *pc;                      // Program counter
    decoded_inst_reg_t *dir;       // Decoded instruction register
    program_t *program;            // Program memory
    bool exception_detected;       // Exception flag from commit stage
		bool backpressure;             // Backpressure flag from rename/dispatch stage
} fetch_decode_t;

/**
 * @brief Initialize the fetch & decode unit
 */
int init_fetch_decode(fetch_decode_t *fetch_decode, 
                     pc_t *pc,
                     decoded_inst_reg_t *dir,
                     program_t *program);

/**
 * @brief Free resources used by the fetch & decode unit
 */
int free_fetch_decode(fetch_decode_t **fetch_decode);

/**
 * @brief Perform one cycle of fetch & decode operations,
 * and increment the program counter by the number of
 * instructions fetched.
 * 
 * @return Number of instructions fetched, a negative value
 *         indicates an error.
 */
int fetch_decode_cycle(fetch_decode_t *fetch_decode);

/**
 * @brief Signal an exception from the commit stage
 */
int signal_exception(fetch_decode_t *fetch_decode);

#endif // FETCH_DECODE_H
