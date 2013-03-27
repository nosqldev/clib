#include <stdio.h>
#include "cfhash.h"
#include "CUnit/Basic.h"

struct item_t
{
    int key;
    int value;
};

HASH_TABLE_DEFINE(intht, 10, item_t, key, value);
hash_ref(intht) g_ht;

static void
test_inner_del(void)
{
    hash_item(intht) item;
    hash_iter(intht) iter;
    hash_ref(intht) ht = hash_new(intht);
    hash_traveler(t);
    int counter = 0;

    g_ht = ht;

#define put(k, v) do { item.key = k; item.value = v; hash_put(intht, ht, &item); } while(0)

    for (int i=0; i<10; i++)
    {
        put(i, i*i);
    }

    item.key = 3;
    hash_del(intht, ht, &item.key);
    CU_ASSERT(ht->occupied_cnt == 9);
    CU_ASSERT(hash_get(intht, ht, &item.key) == NULL);

    while ((iter = hash_travel(intht, ht, t)) != NULL)
    {
        if (counter == 3)
        {
            counter ++;
        }
        CU_ASSERT(iter->key == counter);
        CU_ASSERT(iter->value == counter*counter);
        counter ++;
    }
    CU_ASSERT(counter == 10);

    item.key = 8;
    hash_del(intht, ht, &item.key);
    CU_ASSERT(ht->occupied_cnt == 8);

    counter = 0;
    while ((iter = hash_travel(intht, ht, t)) != NULL)
    {
        if (counter == 3)
        {
            counter ++;
        }
        if (counter == 8)
        {
            counter ++;
        }
        CU_ASSERT(iter->key == counter);
        CU_ASSERT(iter->value == counter*counter);
        counter ++;
    }
    CU_ASSERT(counter == 10);
}

static void
test_head_del(void)
{
    hash_ref(intht) ht = g_ht;
    hash_iter(intht) iter;
    hash_traveler(t);
    int key;

    key = 0;
    hash_del(intht, ht, &key);
    CU_ASSERT(hash_get(intht, ht, &key) == NULL);
    CU_ASSERT(ht->occupied_cnt == 7);

    CU_ASSERT((iter=hash_travel(intht, ht, t)) != NULL);
    CU_ASSERT(iter->key == 1);
    CU_ASSERT(iter->value == 1);
    CU_ASSERT(ht->buckets[t->bucket_pos].links[t->slide_pos].prev_bucket_pos == -1);
    CU_ASSERT(ht->buckets[t->bucket_pos].links[t->slide_pos].prev_slide_pos == -1);
    CU_ASSERT(ht->head_bucket_pos == t->bucket_pos);
    CU_ASSERT(ht->head_slide_pos == t->slide_pos);
    CU_ASSERT(ht->buckets[ht->tail_bucket_pos].items[ht->tail_slide_pos].key == 9);

    CU_ASSERT((iter=hash_travel(intht, ht, t)) != NULL);
    CU_ASSERT(iter->key == 2);
    CU_ASSERT(iter->value == 4);

    CU_ASSERT((iter=hash_travel(intht, ht, t)) != NULL);
    CU_ASSERT(iter->key == 4);
    CU_ASSERT(iter->value == 16);
}

static void
test_tail_del(void)
{
    hash_ref(intht) ht = g_ht;
    hash_iter(intht) iter;
    hash_traveler(t);
    int key;
    int counter = 0;

    key = 9;
    hash_del(intht, ht, &key);
    CU_ASSERT(hash_get(intht, ht, &key) == NULL);
    CU_ASSERT(ht->occupied_cnt == 6);
    while ((iter=hash_travel(intht, ht, t)) != NULL)
    {
        if (counter == 0)
        {
            counter ++;
        }
        if (counter == 3)
        {
            counter ++;
        }
        CU_ASSERT(iter->key == counter);
        CU_ASSERT(iter->value == counter*counter);
        if (iter->key != counter)
        {
            printf("iter->key = %d, counter = %d\n", iter->key, counter);
        }
        counter ++;
    }
    CU_ASSERT(counter == 8);
    CU_ASSERT(ht->buckets[ht->tail_bucket_pos].items[ht->tail_slide_pos].key == 7);
    CU_ASSERT(ht->buckets[ht->tail_bucket_pos].links[ht->tail_slide_pos].next_bucket_pos == -1);
    CU_ASSERT(ht->buckets[ht->tail_bucket_pos].links[ht->tail_slide_pos].next_slide_pos == -1);
}

static void
test_nothing_del(void)
{
    int key = 11;
    CU_ASSERT(hash_del(intht, g_ht, &key) == ERR_HASH_TABLE_NOT_EXISTS);
}

static void
test_all_del(void)
{
    hash_ref(intht) ht = g_ht;
    hash_iter(intht) iter;
    hash_traveler(t);

    while((iter=hash_travel(intht, ht, t)) != NULL)
    {
        CU_ASSERT(hash_del(intht, ht, &iter->key) == 0);
    }
    CU_ASSERT(ht->occupied_cnt == 0);
    CU_ASSERT(ht->head_bucket_pos == ht->tail_bucket_pos);
    CU_ASSERT(ht->head_slide_pos == ht->tail_slide_pos);
    CU_ASSERT(ht->head_bucket_pos == -1);
    CU_ASSERT(ht->head_slide_pos == -1);
}

int
main(void)
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("del test", NULL, NULL);
    if (NULL == pSuite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_inner_del", test_inner_del))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "test_head_del", test_head_del))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "test_tail_del", test_tail_del))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "test_nothing_del", test_nothing_del))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite, "test_all_del", test_all_del))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
