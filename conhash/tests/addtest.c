#include <stdio.h>
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

static void
test_put_int(void)
{
    struct item_int_t item;
    hash_type(intht) *ht = hash_new(intht);

#define put(k, v) do { item.key = k; item.value = v; assert(hash_put(intht, ht, &item) == 0); } while(0);

#if 0
    for (int i=0; i<20; i++)
    {
        put(i, i);
        printf("%d --> %d, %d\n", i, ht->tail_bucket_pos, ht->tail_slide_pos);
    }
#endif

    put(0, 0);
    assert(ht->buckets[0].items[0].key == 0);
    assert(ht->occupied_cnt == 1);
    assert(ht->head_bucket_pos == 0);
    assert(ht->head_slide_pos == 0);
    assert(ht->tail_bucket_pos == 0);
    assert(ht->tail_slide_pos == 0);

    put(0, 0);
    assert(ht->buckets[0].items[0].key == 0);
    assert(ht->occupied_cnt == 1);
    assert(ht->head_bucket_pos == 0);
    assert(ht->head_slide_pos == 0);
    assert(ht->tail_bucket_pos == 0);
    assert(ht->tail_slide_pos == 0);
}

static void
test_put_string(void)
{
}

int
main(void)
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

    test_put_int();

    return 0;
}
