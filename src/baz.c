#include "core.h"
#include "hum_lua.h"
#include "hum_ds.h"
#include "graphics.h"

char funcs[5][256];
double dubs[5];

int test_func(lua_State* L) {

	return 1;
}

SSET_DEFINE(SSetVec2, Vec2);
SSET_DEFINE(SSetInt, int);
SSET_DEFINE(SSetDub, double);


static void test_sparse_sets(void) {
    puts("=== sparse set test ===");

    /* ---------- SSet (owned data) ---------- */
    {
        SSET_DEFINE(SSetInt, int);
        SSetInt *s = SSet_new();

        /* emplace with explicit ids */
        if (!SSet_emplace_back(s, 'a', ((int){ 10 }))) EXIT();
        if (!SSet_emplace_back(s, 'b', ((int){ 20 }))) EXIT();
        if (!SSet_emplace_back(s, 'c', ((int){ 30 }))) EXIT();

        /* dense iteration */
        for (uint32_t i = 0; i < s->internal.len; ++i) {
            printf("SSet int[%u] = %d\n", i, *SSet_at(s, i));
        }

        /* remove middle */
        if (!SSet_remove(s, 'b')) EXIT();
        if (SSet_get(s, 'b') != NULL) EXIT();

        /* id reuse */
        if (!SSet_emplace_back(s, 'b', ((int){ 222 }))) EXIT();
        if (*SSet_get(s, 'b') != 222) EXIT();

        /* push_back path */
        uint32_t id = SSet_push_back(s, ((int){ 999 }));
        if (*SSet_get(s, id) != 999) EXIT();

        printf("SSet final len = %u\n", s->internal.len);

        SSet_free(&s);
    }

    /* ---------- SPSet (pointer-only) ---------- */
    {
        SPSET_DEFINE(SPSetInt, int);
        SPSetInt *s = SPSet_new();

        int a = 1, b = 2, c = 3, d = 4;

        if (!SPSet_emplace_back(s, 'x', &a)) EXIT();
        if (!SPSet_emplace_back(s, 'y', &b)) EXIT();
        if (!SPSet_emplace_back(s, 'z', &c)) EXIT();

        for (uint32_t i = 0; i < s->internal.len; ++i) {
            printf("SPSet ptr[%u] = %d\n", i, *(int *)SPSet_at(s, i));
        }

        /* remove last */
        if (!SPSet_remove(s, 'z')) EXIT();
        if (SPSet_get(s, 'z') != NULL) EXIT();

        /* reuse id */
        if (!SPSet_emplace_back(s, 'z', &d)) EXIT();
        if (*(int *)SPSet_get(s, 'z') != 4) EXIT();

        printf("SPSet final len = %u\n", s->internal.len);

        SPSet_free(&s);
    }

    puts("=== sparse set test OK ===");
}


static void test_DynArr(void) {
    puts("=== DynArr test ===");

    DYNARR_DEFINE(DynArrInt, int);
    DynArrInt *da = DynArr_new();

    /* push a few elements */
    for (int i = 0; i < 10; ++i) {
        uint32_t idx = DynArr_push(da, ((int){ i * 10 }));
        if (idx != (uint32_t)i) EXIT();
    }

    /* check contents */
    for (uint32_t i = 0; i < da->internal.len; ++i) {
        int *v = DynArr_at(da, i);
        if (!v) EXIT();
        if (*v != (int)(i * 10)) EXIT();
        printf("DynArr[%u] = %d\n", i, *v);
    }

    /* grow past initial capacity */
    for (int i = 10; i < 300; ++i) {
        DynArr_push(da, ((int){ i }));
    }

    /* bounds check */
    if (DynArr_at(da, da->internal.len) != NULL) EXIT();

    printf("DynArr final len = %u, cap = %u\n",
           da->internal.len,
           da->internal.cap);

    DynArr_free(&da);

    puts("=== DynArr test OK ===");
}
int main() {
	// test_sparse_sets();
	// test_DynArr();

	Str *st = Str_new();
	Str_put_cstr(st, "peter lustig!");
	StrView view = Str_get_view(st);
	LSView lsview = (LSView) {st->data, 11};

	for (int i = 0; i < 5; i++) {
		for (int i = 0; i < 300; i++) {
			// Str_put_cstr(st, "hallo welt!");
		}
    puts("=== lsview ===");
		LSView_print(&lsview);
    puts("=== view ===");
		StrView_print(&view);
		
	}
	// Str_print(st);
	return 0;

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	if (!lua_check(L, luaL_dofile(L, "src/baz.lua"))) { EXIT(); }

	// SSetVec2 *sset_vec2 = SSet_new();
	SSetInt *sset_int = SSet_new();
	SSetDub *sset_dub = SSet_new();

	if (!SSet_emplace_back(sset_dub, 'i', ((double){1.1}))) { EXIT(); }
	if (!SSet_emplace_back(sset_dub, 'j', ((double){22.22}))) { EXIT(); }

	for (int i = 0; i < sset_dub->internal.len; i++) {
		printf("dub: %f\n", *SSet_at(sset_dub, i));
	}

	if (!SSet_remove(sset_dub, 'j')) { EXIT(); }

	if (!SSet_emplace_back(sset_dub, 'k', ((double){1234.4321}))) { EXIT(); }
	if (!SSet_emplace_back(sset_dub, 'j', ((double){888.999}))) { EXIT(); }

	for (int i = 0; i < sset_dub->internal.len; i++) {
		printf("dub: %f\n", *SSet_at(sset_dub, i));
	}


	// uint32_t id0 = SSet_push_back(sset_vec2, ((Vec2){3.3, 4.4}));
	// uint32_t id1 = SSet_push_back(sset_vec2, ((Vec2){3, 4}));
	bool did = SSet_emplace_back(sset_int, 'k', ((int){88}));
	uint32_t id3 = SSet_push_back(sset_int, ((int){99}));

	if (did) {
		printf("k id: %d\n", *SSet_get(sset_int, 'k'));
	}

	for (int i = 0; i < 300; i++) {
		SSet_push_back(sset_int, ((int){i*10000}));
	}


	return 0;



	// Str str1 = {};
	Str *str2 = Str_new();

	Str_put_cstr(str2, "Panther");
	StrView view2 = Str_get_view(str2);
	StrView_print(&view2);
	for (int i = 0; i < 50; i++) {
		Str_put_cstr(str2, "Panther");
	}
	printf("cap: %d, len: %d\n", str2->cap, str2->len);
	StrView_print(&view2);
	Str_print(str2);

	// Str_put_cstr(&str1, "Backoven");
	// Str_print(&str1);
	// Str_putc(&str1, 'A');
	// Str_print(&str1);
	// printf("HUH: %s\n", str1.data);

	Str_free(str2);
	







	lua_getglobal(L, "bobi");
	if (lua_istable(L, -1)) {
		// if (!lua_access_table(L, "k")) {
		double value = 0;
		char buff[256];
		if (!lua_table_get_string(L, "o", buff, sizeof(buff))) {
			printf("HUH\n");
		} else {
			// printf("value: %f\n", value);
			printf("string: %s\n", buff);
		}
	}

	// --- custom lua interpreter ---
	// char buff[256];
	// // int error = 0;
	// while (fgets(buff, sizeof(buff), stdin) != NULL) {
	// 	// error = luaL_loadbuffer(L, buff, strlen(buff), "line") ||
	// 		// lua_pcall(L, 0, 0, 0);
	// 	lua_check(L, (luaL_loadbuffer(L, buff, strlen(buff), "line") ||
	// 								lua_pcall(L, 0, 0, 0)));
	// }

	lua_close(L);
	return 0;
}
