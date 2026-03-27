// Common

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern const uint32_t INSTRUCTIONS_PER_CYCLE;
#define NUM_ALUS 4

// #define DEBUG

#define RED     "\x1b[31m"
#define RESET   "\x1b[0m"

typedef uint64_t reg_t;

/**
 * Signed register
 */
typedef int64_t  sreg_t;

/**
 * Unsigned register
 */
typedef reg_t    ureg_t;

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static inline char* int64_to_str(int64_t value) {
	char *str = (char *) malloc(21); // 20 digits + null terminator

	if (str != NULL) {
		snprintf(str, 21, "%ld", value);
	}

	return str;
}
static inline char* uint64_to_str(uint64_t value) {
	char *str = (char *) malloc(21); // 20 digits + null terminator

	if (str != NULL) {
		snprintf(str, 21, "%lu", value);
	}

	return str;
}

#endif // COMMON_H
