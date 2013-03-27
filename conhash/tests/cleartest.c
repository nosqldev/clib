#include <stdio.h>
#include "cfhash.h"
#include "CUnit/Basic.h"

struct item_t
{
    int key;
    int value;
};

HASH_TABLE_DEFINE(inthash, 10, item_t, key, value);

static void
test_clear(void)
{
    hash_ref(inthash) ht = hash_new(inthash);
    hash_item(inthash) item;

    for (int i=0; i<100; i++)
    {
        item.key = i;
        item.value = i*i;
        int ret;
        ret = hash_put(inthash, ht, &item);
        if (ret == ERR_HASH_TABLE_PARTIAL_FULL)
        {
            continue;
        }
        else if (ret == ERR_HASH_TABLE_FULL)
        {
            CU_ASSERT(ht->occupied_cnt == ht->buckets_cnt * SLIDE_CNT);
            break;
        }
    }

    for (size_t i=0; i<ht->buckets_cnt; i++)
    {
        CU_ASSERT(ht->buckets[i].used_cnt == 3);
        CU_ASSERT(ht->buckets[i].usage_map[0] == 1);
        CU_ASSERT(ht->buckets[i].usage_map[1] == 1);
        CU_ASSERT(ht->buckets[i].usage_map[2] == 1);
        CU_ASSERT(ht->buckets[i].flag & FLAG_FULL_BUCKET);
    }

    hash_clear(inthash, ht);

    CU_ASSERT(ht->occupied_cnt == 0);
    CU_ASSERT(ht->head_bucket_pos == -1);
    CU_ASSERT(ht->head_slide_pos == -1);
    CU_ASSERT(ht->tail_bucket_pos == -1);
    CU_ASSERT(ht->tail_slide_pos == -1);

    for (size_t i=0; i<ht->buckets_cnt; i++)
    {
        CU_ASSERT(ht->buckets[i].used_cnt == 0);
        CU_ASSERT(ht->buckets[i].usage_map[0] == 0);
        CU_ASSERT(ht->buckets[i].usage_map[1] == 0);
        CU_ASSERT(ht->buckets[i].usage_map[2] == 0);
        CU_ASSERT(ht->buckets[i].flag == 0);
    }
}

int
main(void)
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("clear test", NULL, NULL);
    if (NULL == pSuite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "test_clear", test_clear))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
