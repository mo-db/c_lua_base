#include "core.h"
#include "hum_lua.h"
#include "hum_ds.h"
#include "graphics.h"

char funcs[5][256];
double dubs[5];

int test_func(lua_State* L) {

	return 1;
}

SSET2_DEFINE(SSetVec2, Vec2);
SSET2_DEFINE(SSetInt, int);
SSET2_DEFINE(SSetDub, double);


static void test_sparse_sets(void) {
    puts("=== sparse set test ===");

    /* ---------- SSet2 (owned data) ---------- */
    {
        SSET2_DEFINE(SSetInt, int);
        SSetInt *s = SSet2_new();

        /* emplace with explicit ids */
        if (!SSet2_emplace_back(s, 'a', ((int){ 10 }))) EXIT();
        if (!SSet2_emplace_back(s, 'b', ((int){ 20 }))) EXIT();
        if (!SSet2_emplace_back(s, 'c', ((int){ 30 }))) EXIT();

        /* dense iteration */
        for (uint32_t i = 0; i < s->internal.len; ++i) {
            printf("SSet2 int[%u] = %d\n", i, *SSet2_at(s, i));
        }

        /* remove middle */
        if (!SSet2_remove(s, 'b')) EXIT();
        if (SSet2_get(s, 'b') != NULL) EXIT();

        /* id reuse */
        if (!SSet2_emplace_back(s, 'b', ((int){ 222 }))) EXIT();
        if (*SSet2_get(s, 'b') != 222) EXIT();

        /* push_back path */
        uint32_t id = SSet2_push_back(s, ((int){ 999 }));
        if (*SSet2_get(s, id) != 999) EXIT();

        printf("SSet2 final len = %u\n", s->internal.len);

        SSet2_free(&s);
    }

    /* ---------- SS2 (pointer-only) ---------- */
    {
        SS2_DEFINE(SS2Int, int);
        SS2Int *s = SS2_new();

        int a = 1, b = 2, c = 3, d = 4;

        if (!SS2_emplace_back(s, 'x', &a)) EXIT();
        if (!SS2_emplace_back(s, 'y', &b)) EXIT();
        if (!SS2_emplace_back(s, 'z', &c)) EXIT();

        for (uint32_t i = 0; i < s->internal.len; ++i) {
            printf("SS2 ptr[%u] = %d\n", i, *(int *)SS2_at(s, i));
        }

        /* remove last */
        if (!SS2_remove(s, 'z')) EXIT();
        if (SS2_get(s, 'z') != NULL) EXIT();

        /* reuse id */
        if (!SS2_emplace_back(s, 'z', &d)) EXIT();
        if (*(int *)SS2_get(s, 'z') != 4) EXIT();

        printf("SS2 final len = %u\n", s->internal.len);

        SS2_free(&s);
    }

    puts("=== sparse set test OK ===");
}


static void test_DA2(void) {
    puts("=== DA2 test ===");

    DA2_DEFINE(DA2Int, int);
    DA2Int *da = DA2_new();

    /* push a few elements */
    for (int i = 0; i < 10; ++i) {
        uint32_t idx = DA2_push(da, ((int){ i * 10 }));
        if (idx != (uint32_t)i) EXIT();
    }

    /* check contents */
    for (uint32_t i = 0; i < da->internal.len; ++i) {
        int *v = DA2_at(da, i);
        if (!v) EXIT();
        if (*v != (int)(i * 10)) EXIT();
        printf("DA2[%u] = %d\n", i, *v);
    }

    /* grow past initial capacity */
    for (int i = 10; i < 300; ++i) {
        DA2_push(da, ((int){ i }));
    }

    /* bounds check */
    if (DA2_at(da, da->internal.len) != NULL) EXIT();

    printf("DA2 final len = %u, cap = %u\n",
           da->internal.len,
           da->internal.cap);

    DA2_free(&da);

    puts("=== DA2 test OK ===");
}
int main() {
	test_sparse_sets();
	test_DA2();
	return 0;

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	if (!lua_check(L, luaL_dofile(L, "src/baz.lua"))) { EXIT(); }

	// SSetVec2 *sset_vec2 = SSet2_new();
	SSetInt *sset_int = SSet2_new();
	SSetDub *sset_dub = SSet2_new();

	if (!SSet2_emplace_back(sset_dub, 'i', ((double){1.1}))) { EXIT(); }
	if (!SSet2_emplace_back(sset_dub, 'j', ((double){22.22}))) { EXIT(); }

	for (int i = 0; i < sset_dub->internal.len; i++) {
		printf("dub: %f\n", *SSet2_at(sset_dub, i));
	}

	if (!SSet2_remove(sset_dub, 'j')) { EXIT(); }

	if (!SSet2_emplace_back(sset_dub, 'k', ((double){1234.4321}))) { EXIT(); }
	if (!SSet2_emplace_back(sset_dub, 'j', ((double){888.999}))) { EXIT(); }

	for (int i = 0; i < sset_dub->internal.len; i++) {
		printf("dub: %f\n", *SSet2_at(sset_dub, i));
	}


	// uint32_t id0 = SSet2_push_back(sset_vec2, ((Vec2){3.3, 4.4}));
	// uint32_t id1 = SSet2_push_back(sset_vec2, ((Vec2){3, 4}));
	bool did = SSet2_emplace_back(sset_int, 'k', ((int){88}));
	uint32_t id3 = SSet2_push_back(sset_int, ((int){99}));

	if (did) {
		printf("k id: %d\n", *SSet2_get(sset_int, 'k'));
	}

	for (int i = 0; i < 300; i++) {
		SSet2_push_back(sset_int, ((int){i*10000}));
	}


	Str *st = Str_new();

	if (sset_int) {
		printf("huh\n");
	}

	if (!SSet2_get(sset_int, 400)){
		printf("didnt get\n");
	} else {
		printf("did get\n");
	}

	if (!SSet2_get(sset_int, 200)){
		printf("didnt get\n");
	} else {
		printf("did get\n");
	}


	SSet2_remove(sset_int, id3);
	if (SSet2_get(sset_int, id3)){
		printf("id3: %d\n", *SSet2_get(sset_int, id3));
	}
	SSet2_free(&sset_int);

	

	// for (int i = 0; i < sset_int->internal.len; i++) {
	// 	printf("int: %d\n", *SSet2_at(sset_int, i));
	// }

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
