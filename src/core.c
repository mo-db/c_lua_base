#include "core.h"

bool core_epsilon_equal(double x, double y) {
	return (x < y + epsilon && x > y - epsilon);
}
