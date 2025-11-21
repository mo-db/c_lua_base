#include "core.h"
#include "graphics.h"

int main() {
	Vec2 vtest = {};
	printf("vtest: %f, %f\n", vtest.x, vtest.y);
	mul_Vec2(vtest, 5.0f);
	printf("vtest: %f, %f\n", vtest.x, vtest.y);
	Vec2 vtest2 = {vtest.x + 5, vtest.y + 10};
	printf("vtest2: %f, %f\n", vtest2.x, vtest2.y);
	return 0;
}
