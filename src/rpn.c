#include "rpn.h"

bool _push_back(RPNState* state, double value) {
	if (state->stack_depth >= RPN_STACK_MAX_DEPTH) { return false; }
	state->stack[state->stack_depth++] = value;
	return true;
}
double _pop(RPNState* state, bool* error) {
	if (state->stack_depth == 0) { *error = true; return 0; }
	return state->stack[--state->stack_depth];
}

double eval_rpn(RPNState* state, char* expr, bool* error) {
	*error = false;
	state->stack_depth = 0;

	while (*expr != '\0') {
		if (isspace(*expr)) {
			expr++;
		}

		else if (isdigit(*expr)) {
			char* new_expr_ptr = NULL;
			double value = strtod(expr, &new_expr_ptr);
			if(!_push_back(state, value)) { *error = true; return 0; }
			expr = new_expr_ptr;
		}

		else {
			// unary operations to switch sign
			if (state->stack_depth == 1) {
				double a = _pop(state, error);
				if (*error) { return 0; }
				if (*expr == '-') {
					if(!_push_back(state, a)) { *error = true; return 0; }
				}
				else if (*expr == '+') {
					if(!_push_back(state, a)) { *error = true; return 0; }
				}
				else {
					return DBL_MAX;
				}
			// binary operations
			} else {
				double a = _pop(state, error);
				if (*error) { return 0; }
				double b = _pop(state, error);
				if (*error) { return 0; }

				if (*expr == '+') { 
					if (!_push_back(state, b + a)) { *error = true; return 0; }
				}
				else if (*expr == '-') {
					if (!_push_back(state, b - a)) { *error = true; return 0; }
				}
				else if (*expr == '*') {
					if (!_push_back(state, b * a)) { *error = true; return 0; }
				}
				else if (*expr == '/') {
					if (!_push_back(state, b / a)) { *error = true; return 0; }
				}
				else if (*expr == '^') {
					if (!_push_back(state, pow(b, a))) { *error = true; return 0; }
				}
				else if (*expr == '<') {
					if (*(expr + 1) != '\0' && *(expr + 1) == '=') {
						expr++;
						if (!_push_back(state, b <= a)) { *error = true; return 0; }
					} else {
						if (!_push_back(state, b < a)) { *error = true; return 0; }
					}
				}
				else if (*expr == '>') {
					if (*(expr + 1) != '\0' && *(expr + 1) == '=') {
						expr++;
						if (!_push_back(state, b >= a)) { *error = true; return 0; }
					} else {
						if (!_push_back(state, b > a)) { *error = true; return 0; }
					}
				}
				else if (*expr == '=') {
					if (!_push_back(state, b == a)) { *error = true; return 0; }
				}
				else {
					return DBL_MAX;
				}
			}
			expr++;
		}
	}
	double final_value = _pop(state, error);
	if (*error) { return 0; }
	return final_value;
}
