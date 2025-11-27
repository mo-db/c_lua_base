#include "coroutines_spec.h"
#include "coroutines_impl.h"


void co_init(App* app) {
	ArrList(Test) test_list1 = {};
	// test_list1.internal.cap = 16;
	// test_list1.internal.data = calloc(16, sizeof(Test));




	// ArrList(Test) test_list2 = {};
	bool result = ArrList_alloc(&test_list1, 1024);
	if (result) {
		Test t = (Test){3, 67};
		ArrList_push_back(&test_list1, &t);
		ArrList_push_back(&test_list1, &((Test){9, 44}));

		for (int i = 0; i < ALIST_LEN(test_list1); i++) {
			if (ArrList_at(&test_list1, i)->id == 9) {
				ArrList_remove(&test_list1, i);
			}
		}

		for (int i = 0; i < ALIST_LEN(test_list1); i++) {
			printf("x,y: %d, %d\n", ArrList_at(&test_list1, i)->id,
										ArrList_at(&test_list1, i)->val);
		}
	}





	//
	// Test t = (Test){3, 99};
	// ArrList_push_back(&test_list1, &t);
	// ArrList_push_back(&test_list1, &((Test){9, 44}));
	//
	// printf("Len: %ld\n", ALIST_LEN(test_list1));
	// for (int i = 0; i < ALIST_LEN(test_list1); i++) {
	// 	printf("x,y: %f, %f\n", ArrList_at(&test_list1, i)->position.x,
	// 								ArrList_at(&test_list1, i)->position.x);
	// }

	// bool result = ArrList_remove(&test_list1, 1);
	// if (result) { printf("removed\n"); }


	// ArrList_at(&test_list1, 5)->val = 999;
	// for (int i = 0; i < 16; i++) {
	// 	ArrList_at(&test_list1, i)->id = i;
	// }




	// ArrList(Test)* test_list = new_ArrList(1024, sizeof(Test));
	// ArrList_at(test_list, 0)->a = 5;
	// app->state.co.manips = new_ArrList(1024, sizeof(Manipulator));
	// *ArrList_at(app->state.co.manips, 0) = (Manipulator){ MOVE1, 0, false };
}

void co_update(App* app) {
	CoState* co = &(app->state.co);
	// printf("x coord: %f ",ArrList_at(co->dynamic_objects, 0)->position.x);

	// // run all manipulators
	// for (int i = 0; i < co->manips_count; i++) {
	// 	Manipulator* manip = &(co->manips[i]);
	// 	manip_funcs[manip->manip_type](
	// 			&(co->dynamic_objects[manip->object_id]), 0.0f);
	// }
	//
	// // draw dynamic objects
	// for (int i = 0; i < app->state.co.dynamic_objects_count; i++) {
	// 	Vec2 pos = app->state.co.dynamic_objects[i].position;
	// 	draw_rect(app->my_renderer, pos, add_Vec2(pos, (Vec2){5,5}), 0xFFFFFFFF);
	// }
}
