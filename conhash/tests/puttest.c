#include <stdio.h>
#include "CUnit/Basic.h"
#include "cfhash.h"

struct item_string_t
{
    char key[32];
    char value[32];
};

struct item_int_t
{
    int key;
    int value;
};

HASH_TABLE_DEFINE(stringht, 2000, item_string_t, key, value);
HASH_TABLE_DEFINE(intht, 7, item_int_t, key, value);
HASH_TABLE_DEFINE(fullht, 7, item_int_t, key, value);

#define put_string(k, v) do {\
    strcpy(&string_item.key[0], k); strcpy(&string_item.value[0], v);\
    assert(hash_put(stringht, str_ht, &string_item) == 0);\
}while (0)

#define put2_string(k, v) do {\
    strcpy(&string_item.key[0], k); strcpy(&string_item.value[0], v);\
    assert(hash_put2(stringht, str_ht, &string_item, strlen(k), sizeof(string_item)) == 0);\
}while (0)

#define set_string(k, v) do {\
    assert(hash_set(stringht, str_ht, k, v) == 0);  \
}while (0)

#define set2_string(k, v) do {\
    assert(hash_set2(stringht, str_ht, k, strlen(k), v, strlen(v)) == 0);\
}while (0)

static hash_type(intht) *g_int_ht;

static void
test_put_int(void)
{
    struct item_int_t item;
    hash_type(intht) *ht = hash_new(intht);
    g_int_ht = ht;

#define put(k, v) do { item.key = k; item.value = v; assert(hash_put(intht, ht, &item) == 0); } while(0);

#if 0
    for (int i=0; i<20; i++)
    {
        put(i, i);
        printf("%d --> %d, %d\n", i, ht->tail_bucket_pos, ht->tail_slide_pos);
    }
#endif

    put(0, 0);
    CU_ASSERT(ht->buckets[0].items[0].key == 0);
    CU_ASSERT(ht->buckets[0].items[0].value == 0);
    CU_ASSERT(ht->occupied_cnt == 1);
    CU_ASSERT(ht->head_bucket_pos == 0);
    CU_ASSERT(ht->head_slide_pos == 0);
    CU_ASSERT(ht->tail_bucket_pos == 0);
    CU_ASSERT(ht->tail_slide_pos == 0);

    put(0, 10);
    CU_ASSERT(ht->buckets[0].items[0].key == 0);
    CU_ASSERT(ht->buckets[0].items[0].value == 10);
    CU_ASSERT(ht->occupied_cnt == 1);
    CU_ASSERT(ht->head_bucket_pos == 0);
    CU_ASSERT(ht->head_slide_pos == 0);
    CU_ASSERT(ht->tail_bucket_pos == 0);
    CU_ASSERT(ht->tail_slide_pos == 0);
    CU_ASSERT(ht->buckets[0].links[0].next_bucket_pos == -1);
    CU_ASSERT(ht->buckets[0].links[0].next_slide_pos == -1);

    put(3, 10);
    CU_ASSERT(ht->buckets[1].items[0].key == 3);
    CU_ASSERT(ht->buckets[1].items[0].value == 10);
    CU_ASSERT(ht->occupied_cnt == 2);
    CU_ASSERT(ht->head_bucket_pos == 0);
    CU_ASSERT(ht->head_slide_pos == 0);
    CU_ASSERT(ht->tail_bucket_pos == 1);
    CU_ASSERT(ht->tail_slide_pos == 0);
    CU_ASSERT(ht->buckets[0].links[0].prev_bucket_pos == -1);
    CU_ASSERT(ht->buckets[0].links[0].prev_slide_pos == -1);
    CU_ASSERT(ht->buckets[0].links[0].next_bucket_pos == 1);
    CU_ASSERT(ht->buckets[0].links[0].next_slide_pos == 0);
    CU_ASSERT(ht->buckets[1].links[0].prev_bucket_pos == 0);
    CU_ASSERT(ht->buckets[1].links[0].prev_slide_pos == 0);

    put(13, 100);
    CU_ASSERT(ht->buckets[0].items[0].key == 0);
    CU_ASSERT(ht->buckets[0].items[0].value == 10);
    CU_ASSERT(ht->buckets[0].items[1].key == 13);
    CU_ASSERT(ht->buckets[0].items[1].value == 100);
    CU_ASSERT(ht->occupied_cnt == 3);
    CU_ASSERT(ht->buckets[0].links[1].prev_bucket_pos == 1);
    CU_ASSERT(ht->buckets[0].links[1].prev_slide_pos == 0);
    CU_ASSERT(ht->buckets[0].links[1].next_bucket_pos == -1);
    CU_ASSERT(ht->buckets[0].links[1].next_slide_pos == -1);
    CU_ASSERT(ht->tail_bucket_pos == 0);
    CU_ASSERT(ht->tail_slide_pos == 1);

    put(26, 200);
    CU_ASSERT(ht->buckets[0].items[2].key == 26);
    CU_ASSERT(ht->buckets[0].items[2].value == 200);
    CU_ASSERT(ht->occupied_cnt == 4);
    CU_ASSERT(ht->tail_bucket_pos == 0);
    CU_ASSERT(ht->tail_slide_pos == 2);

    put(39, 1);
    CU_ASSERT(ht->buckets[4].items[0].key == 39);
    CU_ASSERT(ht->buckets[4].items[0].value == 1);
    CU_ASSERT(ht->occupied_cnt == 5);
    CU_ASSERT(ht->tail_bucket_pos == 4);
    CU_ASSERT(ht->tail_slide_pos == 0);
    CU_ASSERT(ht->buckets[0].links[2].next_bucket_pos == 4);
    CU_ASSERT(ht->buckets[0].links[2].next_slide_pos == 0);
    CU_ASSERT(ht->buckets[4].links[0].prev_bucket_pos == 0);
    CU_ASSERT(ht->buckets[4].links[0].prev_slide_pos == 2);
    CU_ASSERT(ht->buckets[4].links[0].next_bucket_pos == -1);
    CU_ASSERT(ht->buckets[4].links[0].next_slide_pos == -1);

    put(12, 2);
    CU_ASSERT(ht->buckets[4].items[1].key == 12);
    CU_ASSERT(ht->buckets[4].items[1].value == 2);
    CU_ASSERT(ht->occupied_cnt == 6);
    CU_ASSERT(ht->tail_bucket_pos == 4);
    CU_ASSERT(ht->tail_slide_pos == 1);
    CU_ASSERT(ht->buckets[4].links[0].next_bucket_pos == 4);
    CU_ASSERT(ht->buckets[4].links[0].next_slide_pos == 1);
    CU_ASSERT(ht->buckets[4].links[1].prev_bucket_pos == 4);
    CU_ASSERT(ht->buckets[4].links[1].prev_slide_pos == 0);
    CU_ASSERT(ht->buckets[4].links[1].next_bucket_pos == -1);
    CU_ASSERT(ht->buckets[4].links[1].next_slide_pos == -1);

    put(52, 3);
    CU_ASSERT(ht->buckets[4].items[2].key == 52);
    CU_ASSERT(ht->buckets[4].items[2].value == 3);
    CU_ASSERT(ht->occupied_cnt == 7);
    CU_ASSERT(ht->tail_bucket_pos == 4);
    CU_ASSERT(ht->tail_slide_pos == 2);
    CU_ASSERT(ht->buckets[4].links[1].next_bucket_pos == 4);
    CU_ASSERT(ht->buckets[4].links[1].next_slide_pos == 2);
    CU_ASSERT(ht->buckets[4].links[2].prev_bucket_pos == 4);
    CU_ASSERT(ht->buckets[4].links[2].prev_slide_pos == 1);

    put(11, 1);
    CU_ASSERT(ht->buckets[8].items[0].key == 11);
    CU_ASSERT(ht->buckets[8].items[0].value == 1);
    CU_ASSERT(ht->occupied_cnt == 8);
    CU_ASSERT(ht->tail_bucket_pos == 8);
    CU_ASSERT(ht->tail_slide_pos == 0);
    CU_ASSERT(ht->buckets[8].links[0].prev_bucket_pos == 4);
    CU_ASSERT(ht->buckets[8].links[0].prev_slide_pos == 2);
    CU_ASSERT(ht->buckets[4].links[2].next_bucket_pos == 8);
    CU_ASSERT(ht->buckets[4].links[2].next_slide_pos == 0);

    put(52, 111);
    CU_ASSERT(ht->buckets[4].items[2].key == 52);
    CU_ASSERT(ht->buckets[4].items[2].value == 111);
    CU_ASSERT(ht->occupied_cnt == 8);
    CU_ASSERT(ht->tail_bucket_pos == 4);
    CU_ASSERT(ht->tail_slide_pos == 2);
    CU_ASSERT(ht->buckets[4].links[1].next_bucket_pos == 8);
    CU_ASSERT(ht->buckets[4].links[1].next_slide_pos == 0);
    CU_ASSERT(ht->buckets[4].links[2].prev_bucket_pos == 8);
    CU_ASSERT(ht->buckets[4].links[2].prev_slide_pos == 0);
    CU_ASSERT(ht->buckets[4].links[2].next_bucket_pos == -1);
    CU_ASSERT(ht->buckets[4].links[2].next_slide_pos == -1);
    CU_ASSERT(ht->buckets[8].links[0].prev_bucket_pos == 4);
    CU_ASSERT(ht->buckets[8].links[0].prev_slide_pos == 1);
    CU_ASSERT(ht->buckets[8].links[0].next_bucket_pos == 4);
    CU_ASSERT(ht->buckets[8].links[0].next_slide_pos == 2);

    put(24, 11);
    CU_ASSERT(ht->buckets[8].items[1].key == 24);
    CU_ASSERT(ht->buckets[8].items[1].value == 11);
    CU_ASSERT(ht->occupied_cnt == 9);
    CU_ASSERT(ht->tail_bucket_pos == 8);
    CU_ASSERT(ht->tail_slide_pos == 1);

    put(37, -12);
    CU_ASSERT(ht->buckets[8].items[2].key == 37);
    CU_ASSERT(ht->buckets[8].items[2].value == -12);
    CU_ASSERT(ht->occupied_cnt == 10);
    CU_ASSERT(ht->tail_bucket_pos == 8);
    CU_ASSERT(ht->tail_slide_pos == 2);

    put(50, 99);
    CU_ASSERT(ht->buckets[12].items[0].key == 50);
    CU_ASSERT(ht->buckets[12].items[0].value == 99);
    CU_ASSERT(ht->occupied_cnt == 11);
    CU_ASSERT(ht->tail_bucket_pos == 12);
    CU_ASSERT(ht->tail_slide_pos == 0);

    put(25, 22);
    CU_ASSERT(ht->buckets[12].items[1].key == 25);
    CU_ASSERT(ht->buckets[12].items[1].value == 22);
    CU_ASSERT(ht->occupied_cnt == 12);
    CU_ASSERT(ht->tail_bucket_pos == 12);
    CU_ASSERT(ht->tail_slide_pos == 1);

    put(65, 0xbeef);
    CU_ASSERT(ht->buckets[12].items[2].key == 65);
    CU_ASSERT(ht->buckets[12].items[2].value == 0xbeef);
    CU_ASSERT(ht->occupied_cnt == 13);
    CU_ASSERT(ht->tail_bucket_pos == 12);
    CU_ASSERT(ht->tail_slide_pos == 2);

    put(37, 9);
    CU_ASSERT(ht->buckets[8].items[2].key == 37);
    CU_ASSERT(ht->buckets[8].items[2].value == 9);
    CU_ASSERT(ht->occupied_cnt == 13);
    CU_ASSERT(ht->tail_bucket_pos == 8);
    CU_ASSERT(ht->tail_slide_pos == 2);
    CU_ASSERT(ht->buckets[8].links[2].prev_bucket_pos == 12);
    CU_ASSERT(ht->buckets[8].links[2].prev_slide_pos == 2);
    CU_ASSERT(ht->buckets[8].links[2].next_bucket_pos == -1);
    CU_ASSERT(ht->buckets[8].links[2].next_slide_pos == -1);
    CU_ASSERT(ht->buckets[12].links[2].next_bucket_pos == 8);
    CU_ASSERT(ht->buckets[12].links[2].next_slide_pos == 2);
    CU_ASSERT(ht->buckets[8].links[1].next_bucket_pos == 12);
    CU_ASSERT(ht->buckets[8].links[1].next_slide_pos == 0);
    CU_ASSERT(ht->buckets[12].links[0].prev_bucket_pos == 8);
    CU_ASSERT(ht->buckets[12].links[0].prev_slide_pos == 1);

    item.key = 78;
    item.value = 9;
    CU_ASSERT(hash_put(intht, ht, &item) == ERR_HASH_TABLE_PARTIAL_FULL);
}

static void
test_travel_put_int(void)
{
    struct item_int_t *int_item_ptr;
    hash_traveler(t);

    int_item_ptr = hash_travel(intht, g_int_ht, t);
    CU_ASSERT(int_item_ptr->key == 0);
    CU_ASSERT(int_item_ptr->value == 10);

#define ASSERT_NEXT(k, v) do { int_item_ptr = hash_travel(intht, g_int_ht, t); CU_ASSERT(int_item_ptr->key == k); CU_ASSERT(int_item_ptr->value == v); } while (0)

    ASSERT_NEXT(3, 10);
    ASSERT_NEXT(13, 100);
    ASSERT_NEXT(26, 200);
    ASSERT_NEXT(39, 1);
    ASSERT_NEXT(12, 2);
    ASSERT_NEXT(11, 1);
    ASSERT_NEXT(52, 111);
    ASSERT_NEXT(24, 11);
    ASSERT_NEXT(50, 99);
    ASSERT_NEXT(25, 22);
    ASSERT_NEXT(65, 0xbeef);
    ASSERT_NEXT(37, 9);
}

static void
test_forward_walk(void)
{
    hash_traveler(t);
    int counter = 0;

    hash_travel(intht, g_int_ht, t);
    CU_ASSERT(t->bucket_pos == 0);
    CU_ASSERT(t->slide_pos == 0);
    counter ++;

#define assert_walk_forward(bpos, spos) do { hash_travel(intht, g_int_ht, t); CU_ASSERT(t->bucket_pos == bpos); CU_ASSERT(t->slide_pos == spos); counter++; } while (0)

    assert_walk_forward(1, 0);
    assert_walk_forward(0, 1);
    assert_walk_forward(0, 2);
    assert_walk_forward(4, 0);
    assert_walk_forward(4, 1);
    assert_walk_forward(8, 0);
    assert_walk_forward(4, 2);
    assert_walk_forward(8, 1);
    assert_walk_forward(12, 0);
    assert_walk_forward(12, 1);
    assert_walk_forward(12, 2);
    assert_walk_forward(8, 2);

    CU_ASSERT(counter == (int)g_int_ht->occupied_cnt);
}

static void
test_backward_walk(void)
{
    int counter = 0;
    int bpos = 0;
    int spos = 0;
    int bpos2;
    int spos2;

    bpos = g_int_ht->tail_bucket_pos;
    spos = g_int_ht->tail_slide_pos;
    
    CU_ASSERT(bpos == 8);
    CU_ASSERT(spos == 2);
    bpos2 = g_int_ht->buckets[bpos].links[spos].prev_bucket_pos;
    spos2 = g_int_ht->buckets[bpos].links[spos].prev_slide_pos;
    bpos = bpos2;
    spos = spos2;
    counter ++;

#define assert_walk_backward(b, s) do {\
    CU_ASSERT(bpos == b);               \
    CU_ASSERT(spos == s);               \
    bpos2 = g_int_ht->buckets[bpos].links[spos].prev_bucket_pos;    \
    spos2 = g_int_ht->buckets[bpos].links[spos].prev_slide_pos; \
    bpos = bpos2;   \
    spos = spos2;   \
    counter ++;     \
} while (0)

    assert_walk_backward(12, 2);
    assert_walk_backward(12, 1);
    assert_walk_backward(12, 0);
    assert_walk_backward(8, 1);
    assert_walk_backward(4, 2);
    assert_walk_backward(8, 0);
    assert_walk_backward(4, 1);
    assert_walk_backward(4, 0);
    assert_walk_backward(0, 2);
    assert_walk_backward(0, 1);
    assert_walk_backward(1, 0);
    assert_walk_backward(0, 0);

    CU_ASSERT((int)g_int_ht->occupied_cnt == counter);
}

static void
test_put_to_full(void)
{
    struct item_int_t item;
    hash_type(fullht) *ht = hash_new(fullht);
    int ret;

    for (int i=0; i<100; i++)
    {
        item.key = i;
        ret = hash_put(fullht, ht, &item);
        if (ret == ERR_HASH_TABLE_PARTIAL_FULL)
        {
            continue;
        }
        else if (ret == ERR_HASH_TABLE_FULL)
        {
            CU_ASSERT(ht->buckets_cnt * 3 == ht->occupied_cnt);
            break;
        }
    }
}

void
test_put_string(void)
{
    struct item_string_t string_item;
    struct item_string_t *string_item_ptr;
    hash_type(stringht) *str_ht = hash_new(stringht);
    hash_traveler(t);

    put_string("1hello", "world");
    put_string("2bai", "du");
    put2_string("3bei", "jing");

    put_string("4bai", "du");
    set_string("5google", "yahoo");
    set2_string("6yes", "no");
    set2_string("7lost", "cs24");

    put2_string("8dou", "ban");

    set_string("9microsoft", "apple");

    while ((string_item_ptr = hash_travel(stringht, str_ht, t)) != NULL)
    {
        assert(string_item_ptr != NULL);
        if (string_item_ptr)
            printf("%s %s\n", string_item_ptr->key, string_item_ptr->value);
        else
            printf("%s [not found]\n", string_item_ptr->key);
    }

    printf("size: %u, buckets_cnt: %u, occupied_cnt: %u\n",
            str_ht->size, str_ht->buckets_cnt, str_ht->occupied_cnt);
}

int
main(void)
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("put_int", NULL, NULL);
    if (NULL == pSuite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "put int", test_put_int))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (CU_add_test(pSuite, "travel put int", test_travel_put_int) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (CU_add_test(pSuite, "forward walk", test_forward_walk) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (CU_add_test(pSuite, "backward walk", test_backward_walk) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (CU_add_test(pSuite, "full put", test_put_to_full) == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
