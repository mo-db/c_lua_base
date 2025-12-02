#include "core.h"

#define RPN_STACK_MAX_DEPTH 64

typedef struct {
	double stack[RPN_STACK_MAX_DEPTH];
	uint32_t stack_depth;
} RPNState;

double eval_rpn(RPNState* state, char* expr, bool* error);
